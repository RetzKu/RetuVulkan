#include "CubeMap.h"
#include <stb_image.h>
#include <minmax.h>

namespace Engine
{
	CubeMap::CubeMap(const char* location, RenderInterface* renderer)
	{
		this->renderer = renderer;
		this->imageBuffer = Buffer(renderer);
		std::vector<uint32_t> indices;
		for (int i = 0; i < 108; i++) { indices.push_back(i); }
		this->cubeBox = new Model(renderer, skyboxVertices, 108, indices);

		CreateTextureImage(location,0);
	}

	CubeMap::~CubeMap()
	{
		
	}

	void CubeMap::CreateTextureImage(const char* file,uint8_t index)
	{
		int width;
		int height;
		int channels;

		std::string result;
		result = file + (std::string)"_ft.tga";
		stbi_uc* pix = stbi_load(result.c_str(),&width,&height,&channels,STBI_rgb_alpha);
		//stbi_uc* pix = stbi_load(result.c_str(),&width,&height,&channels,STBI_rgb_alpha);
		uint32_t mipLevels = 1;

		VkDeviceSize imageSize = width*height*4; //4 because red/green/blue/alpha

		imageBuffer.StartMapping(imageSize*6,VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		imageBuffer.Map(pix, static_cast<size_t>(imageSize));

		result = file + (std::string)"_bk.tga";
		pix = stbi_load(result.c_str(),&width,&height,&channels,STBI_rgb_alpha);
		imageBuffer.Map(pix, static_cast<size_t>(imageSize));

		result = file + (std::string)"_up.tga";
		pix = stbi_load(result.c_str(),&width,&height,&channels,STBI_rgb_alpha);
		imageBuffer.Map(pix, static_cast<size_t>(imageSize));

		result = file + (std::string)"_dn.tga";
		pix = stbi_load(result.c_str(),&width,&height,&channels,STBI_rgb_alpha);
		imageBuffer.Map(pix, static_cast<size_t>(imageSize));
		
		result = file + (std::string)"_rt.tga";
		pix = stbi_load(result.c_str(),&width,&height,&channels,STBI_rgb_alpha);
		imageBuffer.Map(pix, static_cast<size_t>(imageSize));

		result = file + (std::string)"_lf.tga";
		pix = stbi_load(result.c_str(),&width,&height,&channels,STBI_rgb_alpha);
		imageBuffer.Map(pix, static_cast<size_t>(imageSize));

		imageBuffer.StopMapping();

		stbi_image_free(pix);
		CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,image, &imageMemory);
		TransitionImageLayout(image,mipLevels, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(imageBuffer.GetBuffer(), image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		TransitionImageLayout(image,mipLevels, VK_FORMAT_R8G8B8A8_UNORM,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		//GenerateMipLevels(image, width, height, mipLevels);

		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = image;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = mipLevels;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 6;

		if (vkCreateImageView(renderer->logicalDevice, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create test view");
		}
	}

	void CubeMap::CreateImage(int width, int height, VkFormat format, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory* imageMemory)
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 6;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		if(vkCreateImage(renderer->logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(renderer->logicalDevice, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(renderer->physicalDevice, memRequirements.memoryTypeBits, properties);
		
		if (vkAllocateMemory(renderer->logicalDevice,&allocInfo,nullptr, imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory");
		}
		vkBindImageMemory(renderer->logicalDevice, image, *imageMemory, 0);
	}
	VkCommandBuffer CubeMap::beginSingleCommands()
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = *renderer->GetCommandPool(); //what command pool is this
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(renderer->logicalDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}
	void CubeMap::endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(renderer->displayQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(renderer->displayQueue);

		vkFreeCommandBuffers(renderer->logicalDevice, *renderer->GetCommandPool(), 1, &commandBuffer);
	}
	void CubeMap::TransitionImageLayout(VkImage image,uint32_t mipLevels, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = beginSingleCommands();
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 6;

		VkPipelineStageFlags sourceStage = {};
		VkPipelineStageFlags destinationStage = {};

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		//else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL)
		else
		{
			throw std::invalid_argument("unsupported layout transition");
		}

		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}
	void CubeMap::CopyBufferToImage(VkBuffer* buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleCommands();

		VkBufferImageCopy bufferRegions[6];
		int offset = 0;

		for (int i = 0; i < 6; i++)
		{
			bufferRegions[i] = {};
			bufferRegions[i].bufferRowLength = 0;
			bufferRegions[i].bufferImageHeight = 0;
			bufferRegions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferRegions[i].imageSubresource.mipLevel = 0;
			bufferRegions[i].imageSubresource.baseArrayLayer = i;
			bufferRegions[i].imageSubresource.layerCount = 1;
			bufferRegions[i].imageOffset = { 0,0,0 };
			bufferRegions[i].imageExtent = { width,height,1 };

			bufferRegions[i].bufferOffset = offset;
			offset += width * height * 4;
		}

		vkCmdCopyBufferToImage(commandBuffer, *buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, bufferRegions);
		endSingleTimeCommands(commandBuffer);
	}
	void CubeMap::GenerateMipLevels(VkImage image, uint32_t width, uint32_t height, uint32_t mipLevels)
	{
		VkCommandBuffer commandBuffer = beginSingleCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = width;
		int32_t mipHeight = height;

		for (int i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			VkImageBlit blit = {};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;


			vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		}
		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		//barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}
}

