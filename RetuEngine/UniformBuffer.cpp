#include "UniformBuffer.h"

namespace RetuEngine
{
	UniformBuffer::UniformBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue, Camera* camera)
	{
		this->camera = camera;
		Create(logicalDevice, physicalDevice, surface, commandPool, queue);
	}

	UniformBuffer::~UniformBuffer()
	{
	}

	void UniformBuffer::Create(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue)
	{
		VkDeviceSize bufferSize = GetUniformBufferSize();

		CreateBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);
	}

	void UniformBuffer::CleanUp(const VkDevice* logicalDevice)
	{
		CleanUpBuffer(logicalDevice, buffer, bufferMemory);
	}
	void UniformBuffer::Update(const VkDevice* logicalDevice, const VkExtent2D &swapChainExtent)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();

		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo = {  };
		//ubo.Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f, 0.0f));
		ubo.Model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f) , glm::vec3(1.0f, 0.0f, 0.0f));
		//glm::lookat(worldspace of eye,point eye is looking at, which way is upwards)
		//ubo.view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.view = glm::lookAt(camera->cameraPos,  camera->cameraPos + camera->cameraFront, camera->cameraUp);
		ubo.proj = glm::perspective(glm::radians(90.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10000.0f);
		ubo.proj[1][1] *= -1; //flipping y coordinate?

		void* data;
		vkMapMemory(*logicalDevice, bufferMemory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(*logicalDevice, bufferMemory);
	}
}