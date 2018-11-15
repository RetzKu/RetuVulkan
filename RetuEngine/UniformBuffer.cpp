#include "UniformBuffer.h"

namespace RetuEngine
{
	UniformBuffer::UniformBuffer(RenderInterface* renderer)
	{
		this->renderer = renderer;
		Create();
	}

	UniformBuffer::~UniformBuffer()
	{
	}

	void UniformBuffer::Create()
	{
		VkDeviceSize bufferSize = sizeof(glm::mat4);
		CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);
	}

	void UniformBuffer::CleanUp()
	{
		CleanUpBuffer();
	}
	void UniformBuffer::Update()
	{
		//ubo.view = glm::lookAt(camera->cameraPos,  camera->cameraPos + camera->cameraFront, camera->cameraUp);
		//ubo.proj = glm::perspective(glm::radians(90.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10000.0f);
		//ubo.proj[1][1] *= -1; //flipping y coordinate?

		modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f) , glm::vec3(1.0f, 0.0f, 0.0f));

		void* data;
		vkMapMemory(renderer->logicalDevice, bufferMemory, 0, sizeof(glm::mat4), 0, &data);
		memcpy(data, &modelMatrix, sizeof(glm::mat4));
		vkUnmapMemory(renderer->logicalDevice, bufferMemory);
	}
}