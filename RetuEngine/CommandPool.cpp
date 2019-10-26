#include "CommandPool.h"
#include <iostream>

namespace Engine
{
	CommandPool::CommandPool(const VkDevice* logicalDevice, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /* = 0*/)
	{
		CreateCommandPool(logicalDevice, queueFamilyIndex, flags /* = 0*/);
	}


	CommandPool::~CommandPool()
	{
	}

	void CommandPool::CleanUp(const VkDevice* logicalDevice)
	{
		vkDestroyCommandPool(*logicalDevice, commandPool, VK_NULL_HANDLE);
	}

	void CommandPool::CreateCommandPool(const VkDevice* logicalDevice, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /* = 0*/)
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
		commandPoolCreateInfo.flags = flags;

		if (vkCreateCommandPool(*logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to Create CommandPool");
		}
		else
		{
			std::cout << "Created CommandPool successfully" << std::endl;
		}
	}
}