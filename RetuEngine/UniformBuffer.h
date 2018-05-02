#pragma once
#include "Buffer.h"

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
		float nearView = 0.1f;
		float farView = 1000;
	};

	class UniformBuffer : public Buffer
	{
	public:
		struct UniformBufferObject
		{
			glm::mat4 Model;
			glm::mat4 view;
			glm::mat4 proj;
		};

		UniformBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue, Camera* camera);
		~UniformBuffer();

		/*Getters*/
		VkDeviceSize GetUniformBufferSize() { return sizeof(UniformBufferObject);}

		void Create(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue);
		void CleanUp(const VkDevice* logicalDevice);
		void Update(const VkDevice* logicalDevice, const VkExtent2D &swapChainExtent);

		Camera* camera;
	};

}
