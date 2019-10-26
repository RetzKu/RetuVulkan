#pragma once
#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>
#include <string>
#include <vector>
#include <iostream>

/*Engine includes*/
#include "Window.h"
#include "CommandPool.h"

namespace Engine
{
	//const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentmodes;
	};

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

	class RenderInterface
	{
	public:
		RenderInterface(Window* window);
		~RenderInterface();

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice deviceToRate);
		VkCommandPool* GetCommandPool() { return graphicsCommandPool->GetCommandPool(); }

		/*CleanUp Functions*/
		void CleanCommandPools() { graphicsCommandPool->CleanUp(&logicalDevice); transferCommandPool->CleanUp(&logicalDevice); }
		void CleanUp();
		

	/*Private Functions*/
	private:
		void CreateVulkanInstance();
		bool CreateSurface(VkInstance instance, GLFWwindow* window);
		void SetupDebugCallback();
		bool GetPhysicalDevice(VkInstance instance);
		int RateDeviceSuitability(VkPhysicalDevice deviceToRate);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		bool CreateLogicalDevice();
		std::vector<const char*> GetRequiredExtensions();
		bool CheckValidationLayerSupport();

		VkResult CreateDebugReportReportCallbackEXT(
			VkInstance istance,
			const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugReportCallbackEXT* pCallback);

		static void DestoyDebugReportCallbackEXT(
			VkInstance instance,
			VkDebugReportCallbackEXT callback,
			const VkAllocationCallbacks* pAllocator
			);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t obj,
			size_t location,
			int32_t code,
			const char* layerPrefix,
			const char* msg,
			void* userData
		)
		{
			std::cerr << "Validation layer: " << msg << std::endl;
			return VK_FALSE;
		};

	/*public Variables*/
	public:
		const Window* window;
		VkSurfaceKHR surface;
		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
		VkQueue displayQueue;
		VkQueue transferQueue;
		QueueFamilyIndices indices;
		CommandPool* graphicsCommandPool;
		CommandPool* transferCommandPool;

	/*Private Variables*/
	private:
		VkDebugReportCallbackEXT callback;
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

//#ifdef NDEBUG
//		const bool enableValidationLayers = false;
//#else
		const bool enableValidationLayers = true;
//#endif // NDEBUG

	};

}
