#include "Buffer.h"
#include <iostream>
#include "QueueFamily.h"

namespace RetuEngine
{
	void Buffer::CreateBuffer(const VkDevice* logicalDevice,const VkPhysicalDevice* physicalDevice,VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferSize = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(*logicalDevice, &bufferInfo, nullptr,&this->buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(*logicalDevice, this->buffer, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(*physicalDevice,memRequirements.memoryTypeBits,properties);

		if (vkAllocateMemory(*logicalDevice, &allocInfo, nullptr, &this->bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate memory for buffer");
		}
		vkBindBufferMemory(*logicalDevice, this->buffer, this->bufferMemory, 0);
	}


	void Buffer::CreateBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice,const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags)
	{
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;

		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
		if (indices.displayFamily == indices.transferFamily)
		{
			buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			uint32_t indicesArray[] = { static_cast<uint32_t>(indices.displayFamily) };
			buffer_info.pQueueFamilyIndices = indicesArray;
			buffer_info.queueFamilyIndexCount = 1;
		}
		else
		{
			buffer_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
			uint32_t indicesArray[] = { static_cast<uint32_t>(indices.displayFamily), static_cast<uint32_t>(indices.transferFamily) };
			buffer_info.pQueueFamilyIndices = indicesArray;
			buffer_info.queueFamilyIndexCount = 2;
		}
		buffer_info.flags = 0;
		if (vkCreateBuffer(*logicalDevice, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Error while creating buffer");
		}
		else
		{
			std::cout << "buffer created successfully" << std::endl;
		}

		// allocate memory for buffer
		VkMemoryRequirements memory_req;
		vkGetBufferMemoryRequirements(*logicalDevice, buffer, &memory_req);

		VkMemoryAllocateInfo memory_alloc_info = {};
		memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_alloc_info.allocationSize = memory_req.size;
		memory_alloc_info.memoryTypeIndex = findMemoryType(*physicalDevice, memory_req.memoryTypeBits, flags);

		auto memory_result = vkAllocateMemory(*logicalDevice, &memory_alloc_info, nullptr, &bufferMemory);
		if (memory_result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate buffer memory!");
		}

		// bind buffer with memory
		auto bind_result = vkBindBufferMemory(*logicalDevice, buffer, bufferMemory, 0);
		if (bind_result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to bind buffer memory!");
		}
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

	void Buffer::StartMapping(VkDeviceSize bufferSize)
	{
		this->bufferSize = bufferSize;
		CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		vkMapMemory(renderer->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		currentOffset = 0;
	}

	void Buffer::Map(void* newData, VkDeviceSize size)
	{
		memcpy((char*)data + currentOffset, newData, size);
		currentOffset += size;
	}

	void Buffer::StopMapping(VkBufferUsageFlags usage)
	{
		vkUnmapMemory(renderer->logicalDevice, stagingBufferMemory);
		CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
		CopyBuffer(&renderer->logicalDevice, renderer->GetCommandPool(), stagingBuffer, buffer, bufferSize, &renderer->displayQueue);
		vkDestroyBuffer(renderer->logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(renderer->logicalDevice, stagingBufferMemory, nullptr);
		bufferSize;
	}

	void Buffer::CleanUpBuffer(const VkDevice* logicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		vkDestroyBuffer(*logicalDevice, buffer, nullptr);
		vkFreeMemory(*logicalDevice, bufferMemory, nullptr);
	}

	uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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