#pragma once

#include "Common.h"

struct GLFWwindow;

class BaseApp
{
public:
	int Run();

	virtual bool Init(int w, int h) { return true; }
	virtual void Destroy() {}
	virtual void Draw(double elasped) {}

	virtual void FramebufferSizeCallback(GLFWwindow* window, int w, int h);
	virtual void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	virtual void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	virtual void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	virtual void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

};