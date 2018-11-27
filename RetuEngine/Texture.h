#pragma once
#include <vulkan\vulkan.h>
#include "Buffer.h"
#include "RenderInterface.h"

#define MAX_TEXTURE_COUNT 900

namespace RetuEngine
{
	struct pixel
	{
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;
		uint8_t a = 0;
		uint64_t times = 0;
	};

	class Texture
	{
	public:
		Texture(const char* file, RenderInterface* renderer) : imageBuffer(renderer), renderer(renderer) { CreateTextureImage(file); };

		void CleanUp() 
		{ 
			imageBuffer.CleanUpBuffer();
			vkFreeMemory(renderer->logicalDevice, imageMemory,nullptr);
			CleanUpView();
			CleanUpImage();
		};

	private: //Private Functions
		void CleanUpView() { vkDestroyImageView(renderer->logicalDevice,imageView,nullptr); };
		void CleanUpImage() { vkDestroyImage(renderer->logicalDevice, image, nullptr); }

		void CreateTextureImage(const char* file);
		void CreateImage(int width, int height,uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory* imageMemory);
		void TransitionImageLayout(VkImage image,uint32_t mipLevels, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer* buffer, VkImage image, uint32_t width, uint32_t height);
		VkCommandBuffer beginSingleCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void GenerateMipLevels(VkImage image, uint32_t width, uint32_t height, uint32_t miplevels);

	public: //Public Variables
		VkImageView imageView;
		VkImage image;

	private: //Private Variables
		RenderInterface* renderer;
		Buffer imageBuffer;
		VkDeviceMemory imageMemory;
	};

	class TextureVector
	{
	public:
		TextureVector() {};

		void CleanUp()
		{
			for (Texture* var : textures)
			{
				var->CleanUp();
			}
			for (Texture* var : textures)
			{
				delete var;
			}
			textures.clear();
		}

		void Push(const char* name, Texture texture)
		{
			bool taken = false;
			for (std::string var : names)
			{
				if(strcmp(var.c_str(),name) == 0)
				{
					std::cout << "Name taken: " << name << std::endl;
					taken = true;
					break;
				}
			}
			if (!taken)
			{
				textures.push_back(new Texture(texture));
				names.push_back(name);
			}
		}

		Texture* Get(std::string name)
		{
			int index = 0;
			for (std::string var : names)
			{
				if (strcmp(name.c_str(), var.c_str()) == 0)
				{
					return textures[index];
				}
				index++;
			}
			return nullptr;
		}
		
	private:
		std::vector<Texture*> textures;
		std::vector<std::string> names;
		std::vector<int> freeSlots;
	};
}
