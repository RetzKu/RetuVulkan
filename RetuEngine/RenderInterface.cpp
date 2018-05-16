#include "RenderInterface.h"
#include <stdexcept>
#include <iostream>
#include <map>

#define SuccessLog(x) std::cout << x << " created successfully" << std::endl;
#define RunErr(x) throw std::runtime_error("Failed to create" + (std::string)x);

namespace RetuEngine
{
	RenderInterface::RenderInterface(VkInstance instance, GLFWwindow* window)
	{
		CreateSurface(instance, window);
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

	SwapChainSupportDetails RenderInterface::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, nullptr);

		if (presentCount != 0)
		{
			details.presentmodes.resize(presentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, details.presentmodes.data());
		}

		return details;
	}
//const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue
}