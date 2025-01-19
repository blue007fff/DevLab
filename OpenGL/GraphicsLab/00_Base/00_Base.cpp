#include "BaseApp.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h> // 파일 싱크 사용

#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include "Utils.h"


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
        m_camera.Lookat(glm::vec3(2, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

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

class FirstApp : public BaseApp
{
public:
    virtual bool Init(int w, int h) override {
        g_scene.Init();
        g_scene.m_screenSize = glm::ivec2(w, h);

        return true;
    }
    virtual void Destroy() override {
        g_scene.Destory();
	}

	virtual void Draw(double elasped) override {

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
		static utils::FPSCounter fpsCounter;
		fpsCounter.update();
		ImGui::Begin("FPS Display");
		ImGui::Text("Average FPS: %.1lf", fpsCounter.fps());
		ImGui::End();

        g_scene.Draw(elasped);
	}

    void ProcessInput(GLFWwindow* window, double deltaTime)
    {
        if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
            // ImGui가 마우스를 캡처했을 때 3D 화면에 마우스 이벤트를 전달하지 않음
            return;
        }
        m_defaultCameraInputProcessor.Process(g_scene.m_camera, window, deltaTime);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
    }

	virtual void FramebufferSizeCallback(GLFWwindow* window, int w, int h) {
        g_scene.m_screenSize = glm::ivec2(w, h);
	}

	virtual void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
            // ImGui가 마우스를 캡처했을 때 3D 화면에 마우스 이벤트를 전달하지 않음
            return;
        }
	}
	virtual void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
            // ImGui가 마우스를 캡처했을 때 3D 화면에 마우스 이벤트를 전달하지 않음
            return;
        }
	}
	virtual void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
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
	virtual void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) 
    {
        if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
            // ImGui가 마우스를 캡처했을 때 3D 화면에 마우스 이벤트를 전달하지 않음
            return;
        }

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

    DefaultCameraInputProcessor m_defaultCameraInputProcessor;
};

int main()
{
    FirstApp app;
    return app.Run();
}