#include <glad/gl.h>

#include "BaseApp.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h> // 파일 싱크 사용

#include "Camera.h"
//#include "Mesh.h"
//#include "Texture.h"
//#include "ShaderProgram.h"
#include "Utils.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

namespace
{
	bool ImGuIOCapturing()
	{
		// ImGui가 마우스를 캡처했을 때 3D 화면에 마우스 이벤트를 전달하지 않음
		return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
	}

	void BaseAppFramebufferSizeCallbak(GLFWwindow* window, int w, int h)
	{
		// 창에 연결된 유저 데이터 가져오기
		BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
		if (app) { app->FramebufferSizeCallback(window, w, h); }
	}

	void BaseAppMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		if (ImGuIOCapturing()) return;
		BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
		if (app) { app->MouseButtonCallback(window, button, action, mods); }
	}

	void BaseAppCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		if (ImGuIOCapturing()) return;
		BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
		if (app) { app->CursorPosCallback(window, xpos, ypos); }
	}

	void BaseAppScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		if (ImGuIOCapturing()) return;
		BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
		if (app) { app->ScrollCallback(window, xoffset, yoffset); }
	}

	void BaseAppKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (ImGuIOCapturing()) return;
		BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
		if (app) { app->KeyCallback(window, key, scancode, action, mods); }
	}
}


int BaseApp::Run()
{
	if (!glfwInit())
	{
		spdlog::error("Failed to initialize GLFW");
		return -1;
	}

	// OpenGL 버전 3.5 Core Profile 설정
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// GLFW 창 생성
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Base App", nullptr, nullptr);
	if (!window)
	{
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

	// BaseApp 을 유저 데이터로 연결
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, BaseAppFramebufferSizeCallbak);
	glfwSetMouseButtonCallback(window, BaseAppMouseButtonCallback);
	glfwSetCursorPosCallback(window, BaseAppCursorPosCallback);
	glfwSetScrollCallback(window, BaseAppScrollCallback);
	glfwSetKeyCallback(window, BaseAppKeyCallback);

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
	// Second param install_callback=true will install GLFW callbacks 
	// and chain to existing ones.
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	if (Init(width, height))
	{
		double prevTime = glfwGetTime();
		while (!glfwWindowShouldClose(window))
		{
			double currTime = glfwGetTime();
			double deltaTime = currTime - prevTime;
			prevTime = currTime;

			glfwPollEvents();

			ProcessInput(window, deltaTime);

			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			//ImGui::ShowDemoWindow(); // Show demo window! :)

			Draw(deltaTime);

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(window);
		}
	}

	Destroy();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void BaseApp::ProcessInput(GLFWwindow* window, double deltaTime)
{
}

void BaseApp::FramebufferSizeCallback(GLFWwindow* window, int w, int h)
{
}
void BaseApp::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
}
void BaseApp::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
}
void BaseApp::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
}
void BaseApp::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
}

void DefaultCameraInputProcessor::Process(Camera& camera, GLFWwindow* window, double deltaTime)
{
	glm::dvec2 currPos, prevPos;
	glfwGetCursorPos(window, &currPos.x, &currPos.y);
	prevPos = m_lastMousePoint;
	m_lastMousePoint = currPos;

	// 이동 속도는 deltaTime으로 조정
	float velocity = 5.0f * (float)deltaTime;
	float angle_velocity = 45.0f * (float)deltaTime;

	int mouseLState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (mouseLState == GLFW_PRESS)
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		/* g_scene.m_camera.Trackball(
			 g_scene.m_screenSize.x, g_scene.m_screenSize.y,
			 g_scene.m_lastMousePoint, currpos);*/
		
		glm::dvec2 diff = currPos - prevPos;
		camera.Turn(static_cast<float>(diff.x / w * 90.0));
		camera.Lookup(static_cast<float>(-diff.y / h * 90.0));
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.MoveFoward(velocity);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.MoveFoward(-velocity);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.MoveUp(-velocity);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.MoveUp(velocity);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.MoveRight(-velocity);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.MoveRight(velocity);
}