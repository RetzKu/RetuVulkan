#pragma once
#include "Buffer.h"
#include "RenderInterface.h"

#define GLM_FORCE_RADIANS
#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>

#include <chrono>

namespace Engine 
{

	class UniformBuffer : public Buffer
	{
	public:
		UniformBuffer(RenderInterface* renderer) : Buffer(renderer) { Create(); };

		glm::mat4 modelMatrix = {};

		void Create();
		void Update(glm::mat4 objectTransform);
	};
}
