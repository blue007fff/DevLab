#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <iostream>

// 윈도우 크기
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 셰이더 소스 코드
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

void main() 
{
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
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

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        // 셰이더 프로그램 생성 및 링크
        m_shaderID = glCreateProgram();
        glAttachShader(m_shaderID, vertexShader);
        glAttachShader(m_shaderID, fragmentShader);
        glLinkProgram(m_shaderID);

        // 셰이더 삭제
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
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

        // 삼각형 렌더링
        glUseProgram(m_shaderID);

        // x,y,z, r,g,b
        float vertices[] = {
            -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // left-bottom
             0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // right-bottom
             0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f  // top
        };

        // vao
        GLuint vao{ 0 }, vbo{ 0 };
        {
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            // vbo
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            // 버텍스 속성 설정
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // vao 바인딩 해제
            //glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }        

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

public:
    glm::ivec2 m_screenSize{ 0, 0 };
    unsigned int m_shaderID{ 0 };
};

Scene g_scene;

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
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL 버전 3.3 Core Profile 설정
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW 창 생성
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Triangle Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL((GLADloadfunc)(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD!" << std::endl;
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