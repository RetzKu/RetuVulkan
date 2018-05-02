#pragma once
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include "Engine.h"

namespace RetuEngine
{
	class Input
	{
	public:
		Input(GLFWwindow* window, float height, float width, Camera* camera) { m_Window = window; this->height = height; this->width = width; this->camera = camera; }
		~Input() { }
		void UpdateCamera(const double xpos,const double ypos)
		{
			float xoffset = xpos - lookDirection.x;
			float yoffset = lookDirection.y - ypos; // reversed since y-coordinates range from bottom to top
			lookDirection.x = xpos;
			lookDirection.y = ypos;

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
			camera->cameraFront = glm::normalize(front);
		}

		Camera* camera;
		glm::vec3 lookDirection;
		float yaw;
		float pitch;
		GLFWwindow* m_Window;
		float height;
		float width;
	};
}
