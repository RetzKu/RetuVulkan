#include "Window.h"

namespace Engine 
{

	void Camera::UpdateCamera(const double xpos, const double ypos)
	{
		float xoffset = xpos - m_lookdirection.x;
		float yoffset = m_lookdirection.y - ypos; // reversed since y-coordinates range from bottom to top
		m_lookdirection.x = xpos;
		m_lookdirection.y = ypos;

		float sensitivity = 0.08f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;
		yaw += xoffset;
		pitch += yoffset;
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		front.y = sin(glm::radians(pitch));
		front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		cameraFront = glm::normalize(front);
	}

	void Window::Create(Core* core)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		m_window = glfwCreateWindow(width, height, "RetuEngine", nullptr, nullptr);

		glfwSetWindowUserPointer(m_window, core);
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void Window::Resize(int width, int height)
	{
		this->height = height;
		this->width = width;
	}
}