#include "Buffer.h"
#include <iostream>
#include "QueueFamily.h"

namespace RetuEngine
{
	Buffer::Buffer()
	{
	}

	Buffer::~Buffer()
	{
	}

	void Buffer::CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;

		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
		if (indices.displayFamily == indices.transferFamily)
		{
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			uint32_t indicesArray[] = { static_cast<uint32_t>(indices.displayFamily)};
			bufferInfo.pQueueFamilyIndices = indicesArray;
			bufferInfo.queueFamilyIndexCount = 1;
		}
		else 
		{
			bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
			uint32_t indicesArray[] = { static_cast<uint32_t>(indices.displayFamily), static_cast<uint32_t>(indices.transferFamily)};
			bufferInfo.pQueueFamilyIndices = indicesArray;
			bufferInfo.queueFamilyIndexCount = 2;
		}
		if(vkCreateBuffer(*logicalDevice,&bufferInfo,nullptr,&buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Error while creating buffer");
		}
		else
		{
			std::cout << "buffer created successfully" << std::endl;
		}

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(*logicalDevice, buffer, &memReqs);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = findMemoryType(*physicalDevice, memReqs.memoryTypeBits, flags);

		if (vkAllocateMemory(*logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer");
		}
		else
		{
			std::cout << "Allocated buffer successfully" << std::endl;
		}
		vkBindBufferMemory(*logicalDevice, buffer, bufferMemory, 0);
	}
	void Buffer::CleanUpBuffer(const VkDevice* logicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		vkDestroyBuffer(*logicalDevice, buffer, nullptr);
		vkFreeMemory(*logicalDevice, bufferMemory, nullptr);
	}

	uint32_t Buffer::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1<< i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		throw std::runtime_error("Failed to find suitable memory for vertexBuffer");
	}

	void Buffer::CopyBuffer(const VkDevice* logicalDevice, const VkCommandPool* commandPool, VkBuffer src, VkBuffer dst, VkDeviceSize size, const VkQueue* queue)
	{
		VkCommandBufferAllocateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferInfo.commandPool = *commandPool;
		bufferInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(*logicalDevice, &bufferInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;

		vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(*queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(*queue);

		vkFreeCommandBuffers(*logicalDevice, *commandPool, 1, &commandBuffer);
	}
}