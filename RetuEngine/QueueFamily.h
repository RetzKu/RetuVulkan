#pragma once
#include <vulkan\vulkan.h>

namespace RetuEngine
{
	struct QueueFamilyIndices
	{
		int displayFamily = -1;
		int transferFamily = -1;

		bool isComplete()
		{
			return (displayFamily >= 0 && transferFamily >= 0);
		}
	};

	QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice* device,const VkSurfaceKHR* surface);
}
