#pragma once
#include <vulkan\vulkan.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace RetuEngine
{
	class RenderableObject
	{
	public:
		RenderableObject(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue);
		~RenderableObject();

	/*Private Functions*/
	private:
		bool CreateVertexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue);
		bool CreateIndexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue);

	/*Private Variables*/
	private:
		VertexBuffer* vertexBuffer;
		IndexBuffer* indexBuffer;
	};

}
