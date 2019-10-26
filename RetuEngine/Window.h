#pragma once
#include <GLFW\glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace Engine 
{

	class Core; //forward decking CoreClass;
	static void OnWindowResized(GLFWwindow* window, int width, int height);
	static void CursorCallback(GLFWwindow*window, double xpos, double ypos);

	class Camera
	{
	public:

		void UpdateCamera(const double xpos, const double ypos);

		glm::vec3 cameraPos = glm::vec3(0.0f,1.0f,2.0f);
		glm::vec3 cameraFront = glm::vec3(0.0f,0.0f,1.0f);
		glm::vec3 cameraUp = glm::vec3(0.0f,1.0f,0.0f);

		float fieldOfView = 90;
		float nearView = 0.0001f;
		float farView = 1000;

		glm::mat4 view;
		glm::mat4 proj;

	private:
		glm::vec3 m_lookdirection;

		float yaw;
		float pitch;
		float roll;
	};


	class Window
	{
	public:

		void Create(Core* core);
		GLFWwindow* Get() { return m_window; };

		void Resize(int width, int height);

		int height = 800;
		int width = 1000;

	private:

		GLFWwindow* m_window;
	};

}