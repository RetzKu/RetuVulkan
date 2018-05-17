#pragma once
#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>
#include <string>
#include <vector>

/*Engine includes*/
#include "QueueFamily.h"
#include "Window.h"
#include "CommandPool.h"

namespace RetuEngine
{
	//const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentmodes;
	};

	class RenderInterface
	{
	public:
		RenderInterface(VkInstance instace, Window* window);
		~RenderInterface();

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice deviceToRate);
		VkCommandPool* GetCommandPool() { return graphicsCommandPool->GetCommandPool(); }

		/*CleanUp Functions*/
		void CleanCommandPools() { graphicsCommandPool->CleanUp(&logicalDevice); transferCommandPool->CleanUp(&logicalDevice); }
		

	/*Private Functions*/
	private:
		bool CreateSurface(VkInstance instance, GLFWwindow* window);
		bool GetPhysicalDevice(VkInstance instance);
		int RateDeviceSuitability(VkPhysicalDevice deviceToRate);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		bool CreateLogicalDevice();

	/*public Variables*/
	public:
		const Window* window;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
		VkQueue displayQueue;
		VkQueue transferQueue;
		QueueFamilyIndices indices;
		CommandPool* graphicsCommandPool;
		CommandPool* transferCommandPool;

	/*Private Variables*/
	private:
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif // NDEBUG

	};

}
