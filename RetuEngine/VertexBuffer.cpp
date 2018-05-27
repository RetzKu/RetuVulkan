#include "VertexBuffer.h"
#include <iostream>

namespace RetuEngine
{
	VertexBuffer::VertexBuffer(RenderInterface* renderer)
	{
		Create(renderer);
	}

	VertexBuffer::VertexBuffer(RenderInterface* renderer, std::vector<Vertex> customVertices)
	{
		vertices = customVertices;
		Create(renderer);
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	void VertexBuffer::Create(RenderInterface* renderer)
	{
		VkDeviceSize bufferSize = sizeof(vertices[0])* vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(renderer->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(renderer->logicalDevice,stagingBufferMemory);
		CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,buffer,bufferMemory);
		CopyBuffer(&renderer->logicalDevice, renderer->GetCommandPool(), stagingBuffer, buffer, bufferSize, &renderer->displayQueue);
		vkDestroyBuffer(renderer->logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(renderer->logicalDevice, stagingBufferMemory, nullptr);
	}

	void AddToBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue, std::vector<Vertex> newVertices)
	{

	}

	void VertexBuffer::CleanUp(const VkDevice* logicalDevice)
	{
		CleanUpBuffer(logicalDevice,buffer,bufferMemory);
	}
}