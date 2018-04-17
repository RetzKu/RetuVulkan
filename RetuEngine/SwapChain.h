#pragma once

#include <vulkan\vulkan.h>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "Window.h"

namespace RetuEngine
{
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentmodes;
	};

	class SwapChain
	{
	public:
		SwapChain();
		~SwapChain();

		void Create(const VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,Window* windowObj, SwapChainSupportDetails swapChainSupport);
		void CleanUp();
		void CleanUpFrameBuffers();


		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window windowObj);

		void CreateFrameBuffers(VkRenderPass* renderPass);

		void CreateImageViews();

		VkSwapchainKHR* GetSwapchain() { return &swapChain; }
		VkFormat* GetImageFormat() { return &swapChainImageFormat; }
		VkExtent2D* GetExtent() { return &swapChainExtent; }
		size_t  GetFramebufferSize() { return swapChainFramebuffer.size(); }
		VkFramebuffer* GetFramebuffer(unsigned int index) { return &swapChainFramebuffer[index]; }

	private:

		VkSwapchainKHR swapChain;

		std::vector<VkImage> swapChainImages;

		std::vector<VkImageView> swapChainImageViews;

		std::vector<VkFramebuffer> swapChainFramebuffer;

		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		const VkDevice* device;

	};
}

