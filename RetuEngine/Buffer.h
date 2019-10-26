#pragma once
#include <vulkan\vulkan.h>
#include <vector>
#include <iostream>
#include <tuple>
#include "Pointlight.h"
#include "RenderInterface.h"

namespace Engine 
{
	uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	class Buffer
	{
	public:
		Buffer(RenderInterface* renderer) { this->renderer = renderer; };
		Buffer(RenderInterface* renderer, bool useStaging) { this->renderer = renderer; this->useStaging = useStaging; };
		Buffer() {};
		void CleanUpBuffer();


		VkBuffer* GetBuffer() { return &buffer; }
		VkDeviceMemory* GetBufferMemory() { return &bufferMemory; }

		VkDeviceSize bufferSize;
		VkDeviceSize currentOffset;

		void StartMapping(VkDeviceSize bufferSize,VkBufferUsageFlags);
		void Map(void* newData, VkDeviceSize size);
		void StopMapping();

		void StartUpdate();
		void UpdateMap(void* newData, VkDeviceSize size);
		void StopUpdate(VkBufferUsageFlags);

		VkBufferUsageFlags usage;

	protected:

		void* data; //address where data is stored in 
		RenderInterface* renderer;
		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
		VkBuffer stagingBuffer; //we construct staging buffer fully then swap it wth default buffer
		VkDeviceMemory stagingBufferMemory;
		bool useStaging = false;

		void CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
		void CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

		void CopyBuffer(const VkDevice* logicalDevice, const VkCommandPool* commandPool, VkBuffer src, VkBuffer dst, VkDeviceSize size, const VkQueue* queue);
	};
}
