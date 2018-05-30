#pragma once
#include <vulkan\vulkan.h>
#include <vector>
#include <iostream>
#include <tuple>

namespace RetuEngine
{
	uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	class Buffer
	{
	public:
		Buffer();
		~Buffer();
		VkBuffer* GetBuffer() { return &buffer; }
		VkDeviceMemory* GetBufferMemory() { return &bufferMemory; }
		void CreateBuffer(const VkDevice* logicalDevice,const VkPhysicalDevice* physicalDevice,VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		void CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice,const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
		void CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		VkDeviceSize bufferSize;
	protected:
		void CleanUpBuffer(const VkDevice* logicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(const VkDevice* logicalDevice, const VkCommandPool* commandPool, VkBuffer src, VkBuffer dst, VkDeviceSize size, const VkQueue* queue);

	protected:

		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
	};

}
