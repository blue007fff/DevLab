﻿#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>
#include <windows.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h> // 파일 싱크 사용

#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include "Utils.h"


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
    //albedo = vec3(0.5, 0.5, 0.5);
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

        m_shader = gl::ShaderProgram::Create(vertexShaderSource, fragmentShaderSource);
        m_modelLoc = glGetUniformLocation(m_shader->Get(), "u_model");
        m_viewLoc = glGetUniformLocation(m_shader->Get(), "u_view");
        m_projectionLoc = glGetUniformLocation(m_shader->Get(), "u_projection");

        m_lightPosLoc = glGetUniformLocation(m_shader->Get(), "u_lightPos");
        m_lightColorLoc = glGetUniformLocation(m_shader->Get(), "u_lightColor");
        m_viewPosLoc = glGetUniformLocation(m_shader->Get(), "u_viewPos");

        m_shader->Use();
        m_shader->SetUniform(m_lightPosLoc, glm::vec3(3, 3, 3));
        m_shader->SetUniform(m_lightColorLoc, glm::vec3(1.5f, 1.5f, 1.5f));

        // load images
        m_tex0 = gl::Texture::CreateTexture("../../assets/images/awesomeface.png");
        //m_tex1 = Texture::CreateTexture("../../assets/images/container.jpg");
        m_tex1 = gl::Texture::CreateTexture("../../assets/images/debug.jpg");

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        m_qubeMesh = std::make_unique<StaticMesh>(CreateCubeMesh());
        m_qubeMesh->UpdateBuffer();

        m_planeMesh = std::make_unique<StaticMesh>(CreatePlaneMesh());
        m_planeMesh->UpdateBuffer();
    }
    void Destory()
    {
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

        m_shader->Use();
        m_shader->SetUniform(m_viewLoc, viewmat);
        m_shader->SetUniform(m_projectionLoc, projmat);
        glm::vec3 viewpos = glm::vec3(m_camera.GetTransformation()[3]);
        m_shader->SetUniform(m_viewPosLoc, viewpos);

        m_tex0->Bind(0);
        m_tex1->Bind(1);

        {
            glm::mat4 t = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.5f));
            glm::mat4 modelmat = t;
            m_shader->SetUniform(m_modelLoc, modelmat);
            m_qubeMesh->Draw();
        }

        {
            glm::mat4 s = glm::scale(glm::mat4(1), glm::vec3(10));
            glm::mat4 modelmat = s;
            m_shader->SetUniform(m_modelLoc, modelmat);
            m_planeMesh->Draw();
        }
    }

public:
    Camera m_camera;

    glm::vec2 m_lastMousePoint;
    bool m_mousePressed{ false };

    glm::ivec2 m_screenSize{ 0, 0 };
    unsigned int m_shaderID{ 0 };
    std::unique_ptr<gl::ShaderProgram> m_shader;

    GLint m_modelLoc{ 0 };
    GLint m_viewLoc{ 0 };
    GLint m_projectionLoc{ 0 };
    GLint m_lightPosLoc{ 0 };
    GLint m_lightColorLoc{ 0 };
    GLint m_viewPosLoc{ 0 };


    GLuint m_vao{ 0 };
    std::unique_ptr<gl::Texture> m_tex0;
    std::unique_ptr<gl::Texture> m_tex1;
    std::unique_ptr<StaticMesh> m_qubeMesh;
    std::unique_ptr<StaticMesh> m_planeMesh;

}g_scene;

// 콜백 함수: 창 크기 변경 시 호출
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    g_scene.m_screenSize = glm::ivec2(width, height);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// 키 입력 처리
void processInput(GLFWwindow* window, double deltaTime)
{
    if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
        // ImGui가 마우스를 캡처했을 때 3D 화면에 마우스 이벤트를 전달하지 않음
        return;
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
    if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
        // ImGui가 마우스를 캡처했을 때 3D 화면에 마우스 이벤트를 전달하지 않음
        return;
    }

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
    glfwSwapInterval(1);  // 1로 설정하면 V-Sync가 켜짐

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, KeyCallback);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // ImGui 스타일 설정
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");


    int width, height;
    glfwGetWindowSize(window, &width, &height);
    g_scene.Init();
    g_scene.m_screenSize = glm::ivec2(width, height);

    utils::FPSCounter fpsCounter;
    double prevTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        double currTime = glfwGetTime();
        double deltTime = currTime - prevTime;
        prevTime = currTime;

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // Write GUI code
        {
            //ImGui::ShowDemoWindow(); // Show demo window! :)
            //ImGui::ShowUserGuide();

            /*// GUI 코드 작성
            ImGui::Begin("Hello, ImGui!");
            ImGui::Text("This is a simple ImGui example.");
            if (ImGui::Button("Close")) {
                glfwSetWindowShouldClose(window, 1);
            }
            ImGui::End();*/


            // 평균 FPS 계산
            fpsCounter.update();
            ImGui::Begin("FPS Display");
            ImGui::Text("Average FPS: %.1lf", fpsCounter.fps());
            ImGui::End();
        }

        // 입력 처리
        processInput(window, deltTime);

        g_scene.Draw(deltTime);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    g_scene.Destory();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}