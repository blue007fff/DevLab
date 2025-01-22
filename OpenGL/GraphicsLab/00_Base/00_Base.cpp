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
#include "Model.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include "Utils.h"
#include "GLError.h"


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

        namespace fs = std::filesystem;
        fs::path exeFolderPath = utils::GetExecutablePath().parent_path();
        fs::path vsPath = exeFolderPath / "../../assets/shaders/lighting/lighting.vs";
        fs::path fsPath = exeFolderPath / "../../assets/shaders/lighting/lighting.fs";
        m_shader = gl::ShaderProgram::CreateFromFile(vsPath.string(), fsPath.string());
        m_modelLoc = glGetUniformLocation(m_shader->Get(), "u_model");
        m_viewLoc = glGetUniformLocation(m_shader->Get(), "u_view");
        m_projectionLoc = glGetUniformLocation(m_shader->Get(), "u_projection");

        m_lightPosLoc = glGetUniformLocation(m_shader->Get(), "u_lightPos");
        m_lightColorLoc = glGetUniformLocation(m_shader->Get(), "u_lightColor");
        m_viewPosLoc = glGetUniformLocation(m_shader->Get(), "u_viewPos");

        // positions of the point lights
        glm::vec3 pointLightPositions[] = {
            glm::vec3(3, 3, 3),
            glm::vec3(-3, 3, 3),
            glm::vec3(-3, -3, 3),
            glm::vec3(3, -3, 3)
        };

        m_shader->Use();
        m_shader->SetUniform("u_material.shininess", 32.0f);
        m_shader->SetUniform("u_dirLight.direction", glm::vec3(-3));
        m_shader->SetUniform("u_dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        m_shader->SetUniform("u_dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f) * 0.0f);
        m_shader->SetUniform("u_dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f) * 0.0f);

        // point light 1
        m_shader->SetUniform("u_pointLights[0].position", pointLightPositions[0]);
        m_shader->SetUniform("u_pointLights[0].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        m_shader->SetUniform("u_pointLights[0].diffuse", glm::vec3(0.2f, 0.2f, 0.2f) * 0.4f);
        m_shader->SetUniform("u_pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
       m_shader->SetUniform("u_pointLights[0].constant", 1.0f);
       m_shader->SetUniform("u_pointLights[0].linear", 0.09f);
       m_shader->SetUniform("u_pointLights[0].quadratic", 0.032f);
        // point light 2
       m_shader->SetUniform("u_pointLights[1].position", pointLightPositions[1]);
       m_shader->SetUniform("u_pointLights[1].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
       m_shader->SetUniform("u_pointLights[1].diffuse", glm::vec3(1.0f, 0.0f, 0.0f) * 0.4f);
       m_shader->SetUniform("u_pointLights[1].specular", glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
        m_shader->SetUniform("u_pointLights[1].constant", 1.0f);
        m_shader->SetUniform("u_pointLights[1].linear", 0.09f);
        m_shader->SetUniform("u_pointLights[1].quadratic", 0.032f);
        // point light 3
        m_shader->SetUniform("u_pointLights[2].position", pointLightPositions[2]);
        m_shader->SetUniform("u_pointLights[2].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        m_shader->SetUniform("u_pointLights[2].diffuse", glm::vec3(0.0f, 1.0f, 0.0f) * 0.4f);
        m_shader->SetUniform("u_pointLights[2].specular", glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
       m_shader->SetUniform("u_pointLights[2].constant", 1.0f);
       m_shader->SetUniform("u_pointLights[2].linear", 0.09f);
       m_shader->SetUniform("u_pointLights[2].quadratic", 0.032f);
        // point light 4
        m_shader->SetUniform("u_pointLights[3].position", pointLightPositions[3]);
        m_shader->SetUniform("u_pointLights[3].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        m_shader->SetUniform("u_pointLights[3].diffuse", glm::vec3(0.0f, 0.0f, 1.0f) * 0.4f);
        m_shader->SetUniform("u_pointLights[3].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        m_shader->SetUniform("u_pointLights[3].constant", 1.0f);
        m_shader->SetUniform("u_pointLights[3].linear", 0.09f);
        m_shader->SetUniform("u_pointLights[3].quadratic", 0.032f);
        // spotLight
       m_shader->SetUniform("u_spotLight.position", m_camera.GetPos());
       m_shader->SetUniform("u_spotLight.direction", m_camera.GetFoward());
       m_shader->SetUniform("u_spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
       m_shader->SetUniform("u_spotLight.diffuse", glm::vec3(0.3f, 0.3f, 0.0f));
       m_shader->SetUniform("u_spotLight.specular", glm::vec3(1.0f, 0.0f, 1.0f) * 0.3f);
        m_shader->SetUniform("u_spotLight.constant", 1.0f);
        m_shader->SetUniform("u_spotLight.linear", 0.09f);
        m_shader->SetUniform("u_spotLight.quadratic", 0.032f);
        m_shader->SetUniform("u_spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        m_shader->SetUniform("u_spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        m_shader->SetUniform(m_lightPosLoc, -glm::vec3(3, 3, 3));
        //m_shader->SetUniform(m_lightColorLoc, glm::vec3(1));

        gl::error::CheckDriverError();

        // load images
        m_tex0 = gl::Texture::CreateTexture("../../assets/images/awesomeface.png");
        //m_tex1 = Texture::CreateTexture("../../assets/images/container.jpg");
        m_tex1 = gl::Texture::CreateTexture("../../assets/images/debug.jpg");
        m_texDefault = gl::Texture::CreateSingleColorImage(16, 16, glm::vec4(1));
        std::shared_ptr<Material> matDefault(new Material());
        std::shared_ptr<gl::Texture> tex = std::move(m_texDefault);
        matDefault->m_diffuseTex = tex;
        matDefault->m_specularTex = tex;

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        m_qubeMesh = std::make_unique<StaticMesh>(CreateCubeMesh());
        m_qubeMesh->m_material = matDefault;
        m_qubeMesh->UpdateBuffer();

        m_planeMesh = std::make_unique<StaticMesh>(CreatePlaneMesh());
        m_planeMesh->m_material = matDefault;
        m_planeMesh->UpdateBuffer();
        std::filesystem::path modelpath = "../../assets/3d/backpack/backpack.obj";
        std::filesystem::path filePath = exeFolderPath / modelpath;
        m_model = Model::Load(filePath);
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

        m_shader->SetUniform("u_spotLight.position", m_camera.GetPos());
        m_shader->SetUniform("u_spotLight.direction", m_camera.GetFoward());

        m_tex0->Bind(0);
        m_tex1->Bind(1);

        {
            glm::mat4 t = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.5f));
            glm::mat4 modelmat = t;
            m_shader->SetUniform(m_modelLoc, modelmat);
            //m_qubeMesh->Draw();
        }

        {
            glm::mat4 s = glm::scale(glm::mat4(1), glm::vec3(10));
            glm::mat4 modelmat = s;
            m_shader->SetUniform(m_modelLoc, modelmat);
            m_planeMesh->Draw();
        }

        {
            glm::mat4 modelmat = glm::mat4(1);
           /* glm::mat4 modelmat = glm::translate(glm::mat4(1), glm::vec3(0, 2, 0)) 
                * glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1, 0, 0))
                * glm::scale(glm::mat4(1), glm::vec3(1.0f));*/
            m_shader->SetUniform(m_modelLoc, modelmat);
            m_model->Draw(m_shader.get());
        }

        gl::error::CheckDriverError();
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
    std::unique_ptr<gl::Texture> m_texDefault;

    std::unique_ptr<StaticMesh> m_qubeMesh;
    std::unique_ptr<StaticMesh> m_planeMesh;
    std::unique_ptr<Model> m_model;

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