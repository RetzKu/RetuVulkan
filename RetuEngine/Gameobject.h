#pragma once
#include <vulkan\vulkan.hpp>
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "UniformBuffer.h"

namespace RetuEngine
{
	class Gameobject
	{
	public:
		Gameobject(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue);
		~Gameobject();

		glm::vec3 translate;

	private:
		IndexBuffer* indexBuffer;
		VertexBuffer* vertexBuffer;
		UniformBuffer* uniformBuffer;

		const VkDevice* logicalDevice;
	};

}
