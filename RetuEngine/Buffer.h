#pragma once
#include <vulkan\vulkan.h>
#include <vector>
#include <iostream>

namespace RetuEngine
{
	class Buffer
	{
	public:
		Buffer();
		~Buffer();
		VkBuffer* GetBuffer() { return &buffer; }

	protected:
		void CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CleanUpBuffer(const VkDevice* logicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void CopyBuffer(const VkDevice* logicalDevice, const VkCommandPool* commandPool, VkBuffer src, VkBuffer dst, VkDeviceSize size, const VkQueue* queue);

	protected:


		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
	};

}
