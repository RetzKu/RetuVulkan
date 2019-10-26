#pragma once
#include "Window.h"

namespace Engine
{

	class Input
	{
	public:
		Input(Window* window) { m_Window = window->Get(); this->height = window->height; this->width = window->width; this->camera = new Camera(); }
		~Input() { delete camera; }

		void UpdateCamera(const double xpos,const double ypos)
		{
			float xoffset = (float)xpos - lookDirection.x;
			float yoffset = lookDirection.y - (float)ypos; // reversed since y-coordinates range from bottom to top
			lookDirection.x = (float)xpos;
			lookDirection.y = (float)ypos;

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
		int height;
		int width;
	};

}
