#include "RenderableObject.h"



namespace RetuEngine
{
	RenderableObject::RenderableObject(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue)
	{
		CreateVertexBuffer(logicalDevice, physicalDevice, surface, commandPool, queue);
		CreateIndexBuffer(logicalDevice, physicalDevice, surface, commandPool, queue);
	}


	RenderableObject::~RenderableObject()
	{
	}

	bool RenderableObject::CreateIndexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue)
	{
		indexBuffer = new IndexBuffer(logicalDevice, physicalDevice, surface, commandPool, queue);
		if (indexBuffer != nullptr) { throw std::runtime_error("Failed to create Index Buffer"); return true; }
		else { return false; }
	}

	bool RenderableObject::CreateVertexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue)
	{
		vertexBuffer = new VertexBuffer(logicalDevice, physicalDevice, surface, commandPool, queue);
		if (vertexBuffer != nullptr) { throw std::runtime_error("Failed to create Vertex Buffer"); return true; }
		else { return false; }
	}
}