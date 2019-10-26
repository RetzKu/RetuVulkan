#pragma once
#include <vulkan/vulkan.h>
#include "Buffer.h"
#include "Model.h"
#include "RenderInterface.h"

namespace Engine
{

	class CubeMap
	{
	public:
		CubeMap(const char* location, RenderInterface* renderer);
		~CubeMap();
		void CleanUp()
		{
			imageBuffer.CleanUpBuffer();
			vkFreeMemory(renderer->logicalDevice, imageMemory, nullptr);
			vkDestroyImage(renderer->logicalDevice, image, nullptr);
			vkDestroyImageView(renderer->logicalDevice,imageView,nullptr); 
			cubeBox->Delete();
		}

	private:

		void CreateTextureImage(const char* file,uint8_t index);
		void CreateImage(int width, int height, VkFormat format, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory* imageMemory);
		void TransitionImageLayout(VkImage image,uint32_t mipLevels, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer* buffer, VkImage image, uint32_t width, uint32_t height);
		VkCommandBuffer beginSingleCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void GenerateMipLevels(VkImage image, uint32_t width, uint32_t height, uint32_t miplevels);

	public: //Public Variables
		VkImageView imageView;
		VkImage image;
		std::string name;
		Model* cubeBox;

	private: //Private Variables
		RenderInterface* renderer;
		Buffer imageBuffer;
		VkDeviceMemory imageMemory;
		float skyboxVertices[108] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};
	};
}
