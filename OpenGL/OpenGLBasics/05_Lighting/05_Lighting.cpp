﻿#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>  // value_ptr 포함
#include <glm/gtc/quaternion.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <random>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h> // 파일 싱크 사용

#include "Camera.h"
#include "Mesh.h"

std::filesystem::path GetExecutablePath() {
    // Windows에서 실행 파일 경로 얻기
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);  // NULL은 현재 실행 중인 모듈(실행 파일)을 의미
    return std::filesystem::path(buffer);
}

GLuint CreateShader(std::string_view vsCode, std::string_view fsCode);
void CheckShaderCompileErrors(uint32_t shader);
GLuint CreateTexture(std::filesystem::path relativePath);

// 윈도우 크기
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 셰이더 소스 코드
const char* vertexShaderSource = R"(
#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec3 color;
    vec2 texCoord;
} vs_out;

void main() 
{
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0f);

    // calculate world space position and normal
    vs_out.pos = vec3(u_model * vec4(aPos, 1.0));
    // Correct normal for scaling;
    vs_out.normal = mat3(transpose(inverse(u_model))) * aNormal;
    vs_out.color = aColor;
    vs_out.texCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 450 core
in VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec3 color;
    vec2 texCoord;
} fs_in;

// layout(binding = 0): >= 420
layout(binding = 0) uniform sampler2D u_tex0;
layout(binding = 1) uniform sampler2D u_tex1;

// Light properties
uniform vec3 u_lightPos;  // Position of the light source
uniform vec3 u_lightColor; // Color of the light source
uniform vec3 u_viewPos;   // Position of the camera/viewer

out vec4 FragColor;

void main()
{
    vec4 texColor1 = texture(u_tex0, fs_in.texCoord);
    vec4 texColor2 = texture(u_tex1, fs_in.texCoord);
    vec3 albedo =  mix(texColor1, texColor2, 0.5).rgb;
    albedo = mix(fs_in.color, albedo, 0.7);
    //albedo = fs_in.color;
    albedo = vec3(0.5, 0.5, 0.5);
    //FragColor = vec4(albedo, 1.0);
    //return;

    if(!gl_FrontFacing)
        albedo = vec3(0.2, 0.2, 0.8);

    vec3 pos = fs_in.pos;
    vec3 normal = normalize(fs_in.normal);

    vec3 lightColor = u_lightColor;
    vec3 specularColor = vec3(1.0, 1.0, 0.0);
    float specularStrength = 0.5;

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 lightDir = normalize(u_lightPos - pos); // Direction from fragment to light
    float diff = max(dot(normal, lightDir), 0.0); // Lambertian reflectance
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    vec3 viewDir = normalize(u_viewPos - pos); // to view
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * specularColor;

    // Combine results
    vec3 result = (ambient + diffuse) * albedo + specular;
    FragColor = vec4(result, 1.0);
}
)";

class Scene
{
public:
    void Init()
    {
        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK); // Cull the front faces

        m_camera.SetPos(glm::vec3(0, -5, 0));
        glm::quat camquat = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 camtrn = glm::translate(glm::mat4(1), m_camera.GetPos());
        glm::mat4 camrot = glm::mat4_cast(camquat);
        glm::mat4 view = camtrn * camrot;
        m_camera.SetPos(glm::vec3(view[3]));
        m_camera.SetQuat(glm::quat_cast(view));

        m_shaderID = CreateShader(vertexShaderSource, fragmentShaderSource);
        m_modelLoc = glGetUniformLocation(m_shaderID, "u_model");
        m_viewLoc = glGetUniformLocation(m_shaderID, "u_view");
        m_projectionLoc = glGetUniformLocation(m_shaderID, "u_projection");

        m_lightPosLoc = glGetUniformLocation(m_shaderID, "u_lightPos");
        m_lightColorLoc = glGetUniformLocation(m_shaderID, "u_lightColor");
        m_viewPosLoc = glGetUniformLocation(m_shaderID, "u_viewPos");        

        glUseProgram(m_shaderID);
        glUniform3f(m_lightPosLoc, 3.0f, 3.0f, 3.0f);
        glUniform3f(m_lightColorLoc, 1.5f, 1.5f, 1.5f);

        // load images
        stbi_set_flip_vertically_on_load(1);
        m_texID0 = CreateTexture("../../assets/images/awesomeface.png");
        //m_texID1 = CreateTexture("../../assets/images/container.jpg");
        m_texID1 = CreateTexture("../../assets/images/debug.jpg");        

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        m_qubeMesh = CreateCubeMesh();
        m_qubeMesh.UpdateBuffer();

        m_planeMesh = CreatePlaneMesh();
        m_planeMesh.UpdateBuffer();   

        m_cylinderMesh = CreateCylinderMesh(0.5f, 1.0f);
        m_cylinderMesh.UpdateBuffer();

        m_sphereMesh = CreateSphereMesh(0.5f);
        m_sphereMesh.UpdateBuffer();

        m_hsphereMesh = CreateHemisphereMesh(0.5f);
        m_hsphereMesh.UpdateBuffer();

        m_torusMesh = CreateTorusMesh(0.6f, 0.3f);
        m_torusMesh.UpdateBuffer();
    }
    void Destory()
    {
        glDeleteProgram(m_shaderID);
        glDeleteVertexArrays(1, &m_vao);
    }

    void Draw(double elasped)
    {
        float aspect = (float)m_screenSize.x / m_screenSize.y;
        glm::mat4 projmat = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
        //glm::mat4 projmat = glm::perspectiveFovRH_ZO(glm::radians(60.0f), (float)m_screenSize.x, (float)m_screenSize.y, 0.1f, 100.0f);
        glm::mat4 viewmat = m_camera.GetViewMatrix();

        // start draw
        glm::vec4 bufferColor(0.0f, 0.5f, 1.0f, 1.0f);
        float depthValue = 1.0f;
        glClearBufferfv(GL_COLOR, 0, glm::value_ptr(bufferColor));
        glClearBufferfv(GL_DEPTH, 0, &depthValue);
        //glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
        glViewport(0, 0, m_screenSize.x, m_screenSize.y);
        glEnable(GL_DEPTH_TEST);

        glUseProgram(m_shaderID);
        glUniformMatrix4fv(m_viewLoc, 1, GL_FALSE, glm::value_ptr(viewmat));
        glUniformMatrix4fv(m_projectionLoc, 1, GL_FALSE, glm::value_ptr(projmat));
        glm::vec3 viewpos = glm::vec3(m_camera.GetTransformation()[3]);
        glUniform3fv(m_viewPosLoc, 1, glm::value_ptr(viewpos));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texID0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_texID1);

        {
            glm::mat4 t = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.5f));
            glm::mat4 modelmat = t;
            glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, glm::value_ptr(modelmat));
            m_qubeMesh.Draw();
        }

        {
            glm::mat4 t = glm::translate(glm::mat4(1), glm::vec3(0.0f, 2.0f, 0.5f));
            glm::mat4 modelmat = t;
            glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, glm::value_ptr(modelmat));
            m_cylinderMesh.Draw();
        }

        {
            glm::mat4 t = glm::translate(glm::mat4(1), glm::vec3(0.0f, -2.0f, 0.5f));
            glm::mat4 modelmat = t;
            glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, glm::value_ptr(modelmat));
            m_sphereMesh.Draw();
        }

        {
            glm::mat4 t = glm::translate(glm::mat4(1), glm::vec3(2.0f, 0.0f, 0.5f));
            glm::mat4 modelmat = t;
            glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, glm::value_ptr(modelmat));
            m_hsphereMesh.Draw();
        }

        {
            glm::mat4 t = glm::translate(glm::mat4(1), glm::vec3(-2.0f, 0.0f, 0.5f));
            glm::mat4 modelmat = t;
            glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, glm::value_ptr(modelmat));
            m_torusMesh.Draw();
        }


        {
            glm::mat4 s = glm::scale(glm::mat4(1), glm::vec3(10));
            glm::mat4 modelmat = s;
            glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, glm::value_ptr(modelmat));
            m_planeMesh.Draw();
        }        
    }

public:
    Camera m_camera;

    glm::vec2 m_lastMousePoint;
    bool m_mousePressed{ false };

    glm::ivec2 m_screenSize{ 0, 0 };
    unsigned int m_shaderID{ 0 };
    GLint m_modelLoc{ 0 };
    GLint m_viewLoc{ 0 };
    GLint m_projectionLoc{ 0 };
    GLint m_lightPosLoc{ 0 };
    GLint m_lightColorLoc{ 0 };
    GLint m_viewPosLoc{ 0 };


    GLuint m_vao{ 0 };
    GLuint m_texID0{ 0 };
    GLuint m_texID1{ 0 };
    Mesh m_qubeMesh;
    Mesh m_planeMesh;
    Mesh m_cylinderMesh;
    Mesh m_sphereMesh;
    Mesh m_hsphereMesh;
    Mesh m_torusMesh;

}g_scene;

GLuint CreateShader(std::string_view vsCode, std::string_view fsCode)
{
    // 셰이더 컴파일
    GLuint vsID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsID, 1, &vertexShaderSource, nullptr);
    glCompileShader(vsID);
    CheckShaderCompileErrors(vsID);

    GLuint fsID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsID, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fsID);
    CheckShaderCompileErrors(fsID);

    // 셰이더 프로그램 생성 및 링크
    GLuint progID = glCreateProgram();
    glAttachShader(progID, vsID);
    glAttachShader(progID, fsID);
    glLinkProgram(progID);
    CheckShaderCompileErrors(progID);

    // 셰이더 삭제
    glDeleteShader(vsID);
    glDeleteShader(fsID);

    return progID;
}

void CheckShaderCompileErrors(uint32_t shader)
{
    int success;
    char infoLog[1024];
    if (glIsShader(shader))
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            spdlog::error("SHADER_COMPILATION_ERROR: {}", infoLog);
        }
    }
    else if (glIsProgram(shader))
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            spdlog::error("PROGRAM_LINKING_ERROR: {}", infoLog);
        }
    }
}

GLuint CreateTexture(std::filesystem::path relativePath)
{
    GLuint texID{ 0 };

    namespace fs = std::filesystem;
    fs::path exeFolderPath = GetExecutablePath().parent_path();
    fs::path filePath = exeFolderPath / relativePath;

    // 이미지 로드
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath.string().c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum dataFormat = GL_RGB;
        if (nrChannels == 3) dataFormat = GL_RGB;
        else if (nrChannels == 4) dataFormat = GL_RGBA;

        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        // 텍스처 필터링 및 반복 설정
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        spdlog::error("Failed to load texture");
    }
    stbi_image_free(data);
    return texID;
}

// 콜백 함수: 창 크기 변경 시 호출
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    g_scene.m_screenSize = glm::ivec2(width, height);
}

// 키 입력 처리
void processInput(GLFWwindow* window, double deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
   
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

	// 이동 속도는 deltaTime으로 조정
	float velocity = 5.0f * (float)deltaTime;
    float angle_velocity = 45.0f * (float)deltaTime;


	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // 앞으로 이동
		g_scene.m_camera.MoveFoward(velocity);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // 뒤로 이동
		g_scene.m_camera.MoveFoward(-velocity);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) // 왼쪽으로 이동
		g_scene.m_camera.MoveUp(-velocity);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) // 오른쪽으로 이동
		g_scene.m_camera.MoveUp(velocity);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        g_scene.m_camera.MoveRight(-velocity);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        g_scene.m_camera.MoveRight(velocity);
}

// 마우스 버튼 콜백 함수
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{ // 좌클릭
		if (action == GLFW_PRESS)
		{
			g_scene.m_mousePressed = true;
			//glfwGetCursorPos(window, &lastX, &lastY); // 클릭 순간 마우스 위치 저장
		}
		else if (action == GLFW_RELEASE)
		{
            g_scene.m_mousePressed = false;
		}
	}
}

// 마우스 이동 콜백 함수
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    glm::vec2 currpos = glm::vec2(xpos, ypos);
	if (g_scene.m_mousePressed)
    {
       /* g_scene.m_camera.Trackball(
            g_scene.m_screenSize.x, g_scene.m_screenSize.y,
            g_scene.m_lastMousePoint, currpos);*/

        glm::vec2 diff = currpos - g_scene.m_lastMousePoint;
        g_scene.m_camera.Turn(diff.x / (float)g_scene.m_screenSize.x * 90.0f);
        g_scene.m_camera.Lookup(-diff.y / (float)g_scene.m_screenSize.y * 90.0f);
	}
    g_scene.m_lastMousePoint = currpos;
}

// 마우스 휠 스크롤 콜백 함수
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // yoffset 값이 +1이면 휠을 위로, -1이면 아래로 스크롤한 것입니다.
    // xoffset은 수평 스크롤에 대한 정보입니다.
    if (yoffset > 0)
    {
        g_scene.m_camera.Zoom(0.02f);
    }
    else
    {
        g_scene.m_camera.Zoom(-0.02f);
    }
}


int main()
{
    if (!glfwInit()) {
        spdlog::error("Failed to initialize GLFW");
        return -1;
    }

    // OpenGL 버전 3.5 Core Profile 설정
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW 창 생성
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lighting Example", nullptr, nullptr);
    if (!window) {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGL((GLADloadfunc)(glfwGetProcAddress)))
    {
        spdlog::error("Failed to initialize GLAD!");
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetScrollCallback(window, scrollCallback);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    g_scene.Init();
    g_scene.m_screenSize = glm::ivec2(width, height);

    double prevTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        double currTime = glfwGetTime();
        double deltTime = currTime - prevTime;
        prevTime = currTime;

        // 입력 처리
        processInput(window, deltTime);

        g_scene.Draw(deltTime);

        // 버퍼 교환 및 이벤트 처리
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    g_scene.Destory();
    glfwTerminate();
    return 0;
}