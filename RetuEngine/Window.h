#pragma once
#include <GLFW\glfw3.h>

namespace RetuEngine
{
	class Window
	{
	public:
		Window();
		~Window();

		void Create();

		GLFWwindow* window;

		int height = 800;
		int width = 1000;
	};
}