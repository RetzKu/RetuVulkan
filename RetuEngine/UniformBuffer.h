#pragma once
#include "Buffer.h"

#define GLM_FORCE_RADIANS
#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>

#include <chrono>

namespace RetuEngine
{
	class UniformBuffer : public Buffer
	{
	public:
		struct UniformBufferObject
		{
			glm::mat4 Model;
			glm::mat4 view;
			glm::mat4 proj;
		};

		UniformBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue);
		~UniformBuffer();

		/*Getters*/
		VkDeviceSize GetUniformBufferSize() { return sizeof(UniformBufferObject);}

		void Create(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue);
		void CleanUp(const VkDevice* logicalDevice);
		void Update(const VkDevice* logicalDevice, const VkExtent2D &swapChainExtent);
	};

}
