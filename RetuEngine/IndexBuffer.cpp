#include "IndexBuffer.h"



namespace RetuEngine 
{
	IndexBuffer::IndexBuffer(RenderInterface* renderer)
	{
		Create(renderer);
	}
	IndexBuffer::IndexBuffer(RenderInterface* renderer,std::vector<uint32_t> indices)
	{
		this->indices = indices;
		Create(renderer);
	}


	IndexBuffer::~IndexBuffer()
	{
	}

	void IndexBuffer::Create(RenderInterface* renderer)
	{
		VkDeviceSize bufferSize = sizeof(indices[0])* indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(renderer->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(renderer->logicalDevice, stagingBufferMemory);
		CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
		CopyBuffer(&renderer->logicalDevice, renderer->GetCommandPool(), stagingBuffer, buffer, bufferSize, &renderer->displayQueue);
		vkDestroyBuffer(renderer->logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(renderer->logicalDevice, stagingBufferMemory, nullptr);
	}

	void IndexBuffer::CleanUp(const VkDevice* logicalDevice)
	{
		CleanUpBuffer(logicalDevice, buffer, bufferMemory);
	}
}