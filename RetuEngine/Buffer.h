#pragma once
#include <vulkan\vulkan.h>
#include <vector>
#include <iostream>

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
	protected:
		void CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CleanUpBuffer(const VkDevice* logicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(const VkDevice* logicalDevice, const VkCommandPool* commandPool, VkBuffer src, VkBuffer dst, VkDeviceSize size, const VkQueue* queue);

	protected:


		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
	};
}
