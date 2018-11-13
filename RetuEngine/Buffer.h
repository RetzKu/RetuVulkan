#pragma once
#include <vulkan\vulkan.h>
#include <vector>
#include <iostream>
#include <tuple>
#include "Pointlight.h"
#include "RenderInterface.h"

namespace RetuEngine
{
	uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	class Buffer
	{
	public:
		Buffer(RenderInterface* renderer) { this->renderer = renderer; };
		Buffer() {};
		VkBuffer* GetBuffer() { return &buffer; }
		VkDeviceMemory* GetBufferMemory() { return &bufferMemory; }
		void CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

		template<class T>
		void Create(RenderInterface* renderer, VkBufferUsageFlags usage, std::vector<T> data, int amount);

		void CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
		void CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		void* data;
		VkDeviceSize currentOffset;

		void StartMapping(VkDeviceSize bufferSize);
		void Map(void* newData, VkDeviceSize size);
		void StopMapping(VkBufferUsageFlags usage);

		VkDeviceSize bufferSize;


		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		RenderInterface* renderer;

	protected:
		void CleanUpBuffer(const VkDevice* logicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(const VkDevice* logicalDevice, const VkCommandPool* commandPool, VkBuffer src, VkBuffer dst, VkDeviceSize size, const VkQueue* queue);

	
	};

	template<class T>
	void Buffer::Create(RenderInterface* renderer, VkBufferUsageFlags usage, std::vector<T> data, int amount) {
		bufferSize = sizeof(data[0]) * amount;


		CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* dataP;
		vkMapMemory(renderer->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &dataP);
		memcpy(dataP, data.data(), (size_t)bufferSize);
		vkUnmapMemory(renderer->logicalDevice, stagingBufferMemory);
		CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
		CopyBuffer(&renderer->logicalDevice, renderer->GetCommandPool(), stagingBuffer, buffer, bufferSize, &renderer->displayQueue);
		vkDestroyBuffer(renderer->logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(renderer->logicalDevice, stagingBufferMemory, nullptr);
	}

}
