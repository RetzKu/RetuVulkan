#pragma once
#include <vulkan\vulkan.h>
#include "Buffer.h"
#include "RenderInterface.h"

namespace RetuEngine
{
	class Texture
	{
	public:
		Texture(const char* file,RenderInterface* renderer);
		~Texture();

		void CleanUpView() { vkDestroyImageView(renderer->logicalDevice,imageView,nullptr); };
		void CleanUpImage() { vkDestroyImage(renderer->logicalDevice, image, nullptr); }
		void CleanUpMemory() { vkFreeMemory(renderer->logicalDevice, imageMemory, nullptr); }
			

	private: //Private Functions
		void CreateTextureImage(const char* file);
		void CreateImage(int width, int height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		VkCommandBuffer beginSingleCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	public: //Public Variables
		VkImageView imageView;
		VkImage image;
		VkDeviceMemory imageMemory;

	private: //Private Variables
		RenderInterface* renderer;
	};

}
