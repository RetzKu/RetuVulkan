#include "RenderInterface.h"
#include <stdexcept>
#include <iostream>
#include <map>
#include <set>

#define SuccessLog(x) std::cout << x << " created successfully" << std::endl;
#define RunErr(x) throw std::runtime_error("Failed to create" + (std::string)x);

namespace RetuEngine
{
	RenderInterface::RenderInterface(VkInstance instance, Window* window)
	{
		this->window = window;
		CreateSurface(instance, window->window);
		GetPhysicalDevice(instance);
		CreateLogicalDevice();
		indices = FindQueueFamilies(&physicalDevice, &surface);
		graphicsCommandPool = new CommandPool(&logicalDevice,indices.displayFamily);
		transferCommandPool = new CommandPool(&logicalDevice,indices.transferFamily,VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	}

	RenderInterface::~RenderInterface()
	{
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
}