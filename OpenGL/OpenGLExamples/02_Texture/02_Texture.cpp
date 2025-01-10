#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <filesystem>
#include <windows.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h> // 파일 싱크 사용

std::filesystem::path GetExecutablePath() {
    // Windows에서 실행 파일 경로 얻기
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);  // NULL은 현재 실행 중인 모듈(실행 파일)을 의미
    return std::filesystem::path(buffer);
}

void CheckShaderCompileErrors(uint32_t shader);
GLuint CreateTexture(std::filesystem::path relativePath);

// 윈도우 크기
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 셰이더 소스 코드
const char* vertexShaderSource = R"(
#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out VS_OUT
{
    vec3 color;
    vec2 texCoord;
} vs_out;


void main() 
{
    gl_Position = vec4(aPos, 1.0);
    vs_out.color = aColor;
    vs_out.texCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 450 core
in VS_OUT
{
    vec3 color;
    vec2 texCoord;
} fs_in;

// layout(binding = 0): >= 420
layout(binding = 0) uniform sampler2D u_tex0;
layout(binding = 1) uniform sampler2D u_tex1;

out vec4 FragColor;

void main()
{
    vec4 texColor1 = texture(u_tex0, fs_in.texCoord);
    vec4 texColor2 = texture(u_tex1, fs_in.texCoord);

    FragColor = mix(texColor1, texColor2, 0.5);
}
)";

class Scene
{
public:
    void Init()
    {
        // 셰이더 컴파일
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);
        CheckShaderCompileErrors(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);
        CheckShaderCompileErrors(fragmentShader);

        // 셰이더 프로그램 생성 및 링크
        m_shaderID = glCreateProgram();
        glAttachShader(m_shaderID, vertexShader);
        glAttachShader(m_shaderID, fragmentShader);
        glLinkProgram(m_shaderID);
        CheckShaderCompileErrors(m_shaderID);

        // 셰이더 삭제
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // load images
        stbi_set_flip_vertically_on_load(1);
        m_texID0 = CreateTexture("../../assets/images/awesomeface.png");
        m_texID1 = CreateTexture("../../assets/images/container.jpg");
    }
    void Destory()
    {
        glDeleteProgram(m_shaderID);
    }

	void Draw()
	{
		// 화면 지우기
		glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, m_screenSize.x, m_screenSize.y);

		glUseProgram(m_shaderID);

		// x,y,z, r,g,b
		struct Vertex
		{
			glm::vec3 pos;
			glm::vec3 color;
			glm::vec2 texCoord;
		};
		Vertex vertices[]{
			{{-0.5f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // 왼쪽 위
			{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 왼쪽 아래
			{{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 오른쪽 아래
			{{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 오른쪽 위
		};
		GLuint indices[] = {
			0, 1, 2, // 첫 번째 삼각형
			0, 2, 3  // 두 번째 삼각형
		};

		// vao
		GLuint vao{ 0 }, vbo{ 0 }, ebo{ 0 };
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			// vbo
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glGenBuffers(1, &ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

			// 버텍스 속성 설정
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
			glEnableVertexAttribArray(2);

			// vao 바인딩 해제
			//glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texID0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_texID1);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);
		glDeleteVertexArrays(1, &vao);
	}

public:
	glm::ivec2 m_screenSize{ 0, 0 };
	unsigned int m_shaderID{ 0 };
	GLuint m_texID0{ 0 };
	GLuint m_texID1{ 0 };
}g_scene;

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

void CheckShaderCompileErrors(uint32_t shader)
{
    int success;
    char infoLog[1024];
    if(glIsShader(shader))
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

// 콜백 함수: 창 크기 변경 시 호출
void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
    g_scene.m_screenSize = glm::ivec2(width, height);
}

// 키 입력 처리
void processInput(GLFWwindow* window) 
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Texture Example", nullptr, nullptr);
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

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    g_scene.Init();
    g_scene.m_screenSize = glm::ivec2(width, height);

    // 렌더링 루프
    while (!glfwWindowShouldClose(window))
    {
        // 입력 처리
        processInput(window);

        g_scene.Draw();

        // 버퍼 교환 및 이벤트 처리
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    g_scene.Destory();
    glfwTerminate();
    return 0;
}