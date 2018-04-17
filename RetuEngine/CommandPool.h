#pragma once

#include <vulkan\vulkan.h>
namespace RetuEngine
{
	class CommandPool
	{
	public:
		CommandPool(const VkDevice* logicalDevice, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0/* = 0*/);
		~CommandPool();

		void CleanUp(const VkDevice* logicalDevice);

		VkCommandPool* GetCommandPool() { return &commandPool; }
	private:
		void CreateCommandPool(const VkDevice* logicalDevice, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);
		VkCommandPool commandPool;
	};
}
