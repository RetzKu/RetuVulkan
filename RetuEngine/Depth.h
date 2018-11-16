#pragma once
#include <vector>
#include <vulkan\vulkan.h>
#include "Buffer.h"
#include "RenderInterface.h"

namespace RetuEngine
{
	class Depth
	{
	public:
		Depth(RenderInterface* renderer,VkExtent2D extent);
		void CleanUp()
		{
			CleanUpView();
			CleanUpImage();
			vkFreeMemory(renderer->logicalDevice, depthImageMemory, nullptr);
		};

	private: //Private funcitons
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		void CreateImage(VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkFormat FindDepthFormat() { return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT,VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT); }
		bool HasStencilComponent(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
		void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		VkCommandBuffer beginSingleCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CleanUpView() { vkDestroyImageView(renderer->logicalDevice,depthImageView,nullptr); };
		void CleanUpImage() { vkDestroyImage(renderer->logicalDevice, depthImage, nullptr); }

	public: //Public variables
		VkFormat depthFormat;
		VkImage depthImage;
		VkImageView depthImageView;

	private: //private variables
		VkDeviceMemory depthImageMemory;
		RenderInterface* renderer;
	};
}
