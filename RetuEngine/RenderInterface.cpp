#include "RenderInterface.h"
#include <stdexcept>
#include <iostream>
#include <map>
#include <set>

#define SuccessLog(x) std::cout << x << " created successfully" << std::endl;
#define RunErr(x) throw std::runtime_error("Failed to create" + (std::string)x);

namespace Engine
{
	RenderInterface::RenderInterface(Window* window)
	{
		this->window = window;
		CreateVulkanInstance();
		SetupDebugCallback();
		CreateSurface(instance, window->Get());
		GetPhysicalDevice(instance);
		CreateLogicalDevice();
		indices = FindQueueFamilies(&physicalDevice, &surface);
		graphicsCommandPool = new CommandPool(&logicalDevice,indices.displayFamily);
		transferCommandPool = new CommandPool(&logicalDevice,indices.transferFamily,VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	}

	RenderInterface::~RenderInterface()
	{
	}

	void RenderInterface::CreateVulkanInstance()
	{
		if (enableValidationLayers && !CheckValidationLayerSupport())
		{
			throw std::runtime_error("Validation layer requested but are not available");
		}
		VkApplicationInfo appInfo = {};
		
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "VulkanApp";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;
		
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &appInfo;

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}

		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		if (result)
		{
			throw std::runtime_error("Failed to create vulkan instance");
		}
		else
		{
			std::cout << "Vulkan instance created successfully" << std::endl;
		}
	}

	void RenderInterface::CleanUp()
	{
		CleanCommandPools();

		vkDestroyDevice(logicalDevice, VK_NULL_HANDLE);
		DestoyDebugReportCallbackEXT(instance, callback, VK_NULL_HANDLE);
		vkDestroySurfaceKHR(instance, surface, VK_NULL_HANDLE);
		vkDestroyInstance(instance, VK_NULL_HANDLE);
	}

	bool RenderInterface::CreateSurface(VkInstance instance, GLFWwindow* window)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			RunErr("Surface");
			return false;
		}
		else
		{
			SuccessLog("Surface");
			return true;
		}
	}

	bool RenderInterface::GetPhysicalDevice(VkInstance instance)
	{
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		if (physicalDeviceCount == 0)
		{
			throw std::runtime_error("No devices with Vulkan support");
		}
		std::vector<VkPhysicalDevice> foundPhysicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, foundPhysicalDevices.data());

		std::multimap<int, VkPhysicalDevice> rankedDevices;

		for (const auto& currentDevice : foundPhysicalDevices)
		{
			int score = RateDeviceSuitability(currentDevice);
			rankedDevices.insert(std::make_pair(score, currentDevice));
		}

		if (rankedDevices.rbegin()->first > 0)
		{
			physicalDevice = rankedDevices.rbegin()->second;
			std::cout << "PhysicalDevice Found" << std::endl;
		}
		else
		{
			throw std::runtime_error("No Physical devices that support Vulkan");
		}
		return true;
	}

	int RenderInterface::RateDeviceSuitability(VkPhysicalDevice deviceToRate)
	{
		int score = 0;

		QueueFamilyIndices indices = FindQueueFamilies(&deviceToRate, &surface);
		bool extensionsSupported = CheckDeviceExtensionSupport(deviceToRate);
		if (indices.isComplete() == false || !extensionsSupported)
		{
			return 0;
		}

		bool swapChainAdequate = false;
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(deviceToRate);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentmodes.empty();
		if (!swapChainAdequate)
		{
			return 0;
		}

		VkPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(deviceToRate, &deviceProperties);
		vkGetPhysicalDeviceFeatures(deviceToRate, &deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		score += deviceProperties.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader)
		{
			return 0;
		}
		return score;
	}

	bool RenderInterface::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensioncount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensioncount, nullptr);

		std::vector<VkExtensionProperties> availableExtension(extensioncount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensioncount, availableExtension.data());

		for (const char* currentextension : deviceExtensions)
		{
			bool extensionFound = false;
			for (const auto& extension : availableExtension)
			{
				if (strcmp(currentextension, extension.extensionName) == 0)
				{
					extensionFound = true;
				}
			}
			if (!extensionFound)
			{
				return false;
			}
		}
		return true;
	}

	SwapChainSupportDetails RenderInterface::QuerySwapChainSupport(VkPhysicalDevice deviceToRate)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceToRate, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(deviceToRate, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(deviceToRate, surface, &formatCount, details.formats.data());
		}

		uint32_t presentCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(deviceToRate, surface, &presentCount, nullptr);

		if (presentCount != 0)
		{
			details.presentmodes.resize(presentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(deviceToRate, surface, &presentCount, details.presentmodes.data());
		}

		return details;
	}

	bool RenderInterface::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(&physicalDevice, &surface);
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.displayFamily, indices.transferFamily };
		const float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}


		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}
		createInfo.enabledExtensionCount = deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device");
		}
		else
		{
			std::cout << "Logical device created successfully" << std::endl;
		}
		vkGetDeviceQueue(logicalDevice, indices.displayFamily, 0, &displayQueue);
		vkGetDeviceQueue(logicalDevice, indices.transferFamily, 0, &transferQueue);
	}

	bool RenderInterface::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount,nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		for (const char* layername : validationLayers)
		{
			bool layerFound = false;
			for (const auto &layerProperties : availableLayers)
			{
				if (strcmp(layername, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}
			if (!layerFound) { return false; }
		}
		return true;
	}

	std::vector<const char*> RenderInterface::GetRequiredExtensions()
	{
		std::vector<const char*> extensions;
		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		for (unsigned int i = 0; i < glfwExtensionCount; i++)
		{
			extensions.push_back(glfwExtensions[i]);
		}
		if (enableValidationLayers) { extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); }
		return extensions;
	}

	void RenderInterface::SetupDebugCallback()
	{
		if (!enableValidationLayers)
		{
			return;
		}
		VkDebugReportCallbackCreateInfoEXT createInfo = {  };
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (CreateDebugReportReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to setup debug callback");
		}
		else
		{
			std::cout << "Debug callback setup successfull" << std::endl;
		}
	}

	VkResult RenderInterface::CreateDebugReportReportCallbackEXT( VkInstance istance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) { return func(instance, pCreateInfo, pAllocator, pCallback); }
		else { return VK_ERROR_EXTENSION_NOT_PRESENT; }
	}

	void RenderInterface::DestoyDebugReportCallbackEXT( VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr)
		{
			func(instance, callback, pAllocator);
		}
	}

	QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice* device,const VkSurfaceKHR* surface)
	{
		QueueFamilyIndices indices;
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(*device, i, *surface, &presentSupport);

			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport)
			{
				indices.displayFamily = i;
			}

			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport)
			{
				indices.transferFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}
			i++;
		}
		if(indices.displayFamily >= 0 && indices.transferFamily == -1)
		{
			indices.transferFamily = indices.displayFamily;
		}
		return indices;
	}

}