#include "VertexBuffer.h"
#include <iostream>

namespace RetuEngine
{
	VertexBuffer::VertexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue)
	{
		Create(logicalDevice, physicalDevice, surface, commandPool, queue);
	}

	VertexBuffer::VertexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue, std::vector<Vertex> customVertices)
	{
		vertices = customVertices;
		Create(logicalDevice, physicalDevice, surface, commandPool, queue);
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	void VertexBuffer::Create(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue)
	{
		VkDeviceSize bufferSize = sizeof(vertices[0])* vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingBuffer,stagingBufferMemory);

		void* data;
		vkMapMemory(*logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(*logicalDevice,stagingBufferMemory);
		CreateBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,buffer,bufferMemory);
		CopyBuffer(logicalDevice, commandPool, stagingBuffer, buffer, bufferSize, queue);
		vkDestroyBuffer(*logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(*logicalDevice, stagingBufferMemory, nullptr);
	}

	//void VertexBuffer::Create(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue)
	//{
	//	VkDeviceSize bufferSize = sizeof(vertices[0])* vertices.size();

	//	VkBuffer stagingBuffer;
	//	VkDeviceMemory stagingBufferMemory;

	//	CreateBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingBuffer,stagingBufferMemory);

	//	void* data;
	//	vkMapMemory(*logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	//	memcpy(data, vertices.data(), (size_t)bufferSize);
	//	vkUnmapMemory(*logicalDevice,stagingBufferMemory);
	//	CreateBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,buffer,bufferMemory);
	//	CopyBuffer(logicalDevice, commandPool, stagingBuffer, buffer, bufferSize, queue);
	//	vkDestroyBuffer(*logicalDevice, stagingBuffer, nullptr);
	//	vkFreeMemory(*logicalDevice, stagingBufferMemory, nullptr);
	//}

	void AddToBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue, std::vector<Vertex> newVertices)
	{

	}

	void VertexBuffer::CleanUp(const VkDevice* logicalDevice)
	{
		CleanUpBuffer(logicalDevice,buffer,bufferMemory);
	}
}