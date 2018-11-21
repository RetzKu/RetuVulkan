#pragma once
#include "Buffer.h"
#include "RenderInterface.h"

#define GLM_FORCE_RADIANS
#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>

#include <chrono>

namespace RetuEngine
{
	struct Camera
	{
		glm::vec3 cameraPos = glm::vec3(0.0f,1.0f,2.0f);
		glm::vec3 cameraFront = glm::vec3(0.0f,0.0f,-1.0f);
		glm::vec3 cameraUp = glm::vec3(0.0f,1.0f,0.0f);
		float fieldOfView = 90;
		float nearView = 0.0001f;
		float farView = 1000;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class UniformBuffer : public Buffer
	{
	public:
		UniformBuffer(RenderInterface* renderer) : Buffer(renderer) { Create(); };

		glm::mat4 modelMatrix = {};

		void Create();
		void Update(glm::mat4 objectTransform);
	};
}
