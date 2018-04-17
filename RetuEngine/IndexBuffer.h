#pragma once
#include "Buffer.h"

namespace RetuEngine
{
	class IndexBuffer : public Buffer
	{
	public:
		IndexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue);
		~IndexBuffer();

		void Create(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue);
		void CleanUp(const VkDevice* logicalDevice);

		uint32_t GetIndicesSize() { return indices.size(); }

	protected:
		const std::vector<uint32_t> indices =
		{
			0,1,2,2,3,0
		};
	};

}
