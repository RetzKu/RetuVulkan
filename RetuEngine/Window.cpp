#include "Window.h"

namespace RetuEngine
{
	Window::Window()
	{
	}

	Window::~Window()
	{
	}

	void Window::Create()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window = glfwCreateWindow(width, height, "RetuEngine", nullptr, nullptr);
	}
}