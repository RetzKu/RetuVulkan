#pragma once
#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>
#include <string>
#include <vector>

/*Engine includes*/
#include "QueueFamily.h"
#include "SwapChain.h"

namespace RetuEngine
{
	//const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue
	class RenderInterface
	{
	public:
		RenderInterface(VkInstance instace, GLFWwindow* window);
		~RenderInterface();

	/*Private Functions*/
	private:
		bool CreateSurface(VkInstance instance, GLFWwindow* window);
		bool GetPhysicalDevice(VkInstance instance);
		int RateDeviceSuitability(VkPhysicalDevice deviceToRate);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

	/*public Variables*/
	public:
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;

	/*Private Variables*/
	private:
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	};

}
