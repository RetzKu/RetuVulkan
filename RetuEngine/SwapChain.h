#pragma once
#include <vulkan\vulkan.h>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "Window.h"
#include "Depth.h"
#include "RenderInterface.h"

namespace Engine
{

	class SwapChain
	{
	public:
		~SwapChain();

		void Create(RenderInterface* renderer);
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
		VkFormat* GetDepthFormat() { return &depth->depthFormat; }

	private:
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffer;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		const VkDevice* device;
		Depth* depth;
	};
}

