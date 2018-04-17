#include "Gameobject.h"



namespace RetuEngine
{
	Gameobject::Gameobject(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue)
	{
		vertexBuffer = new VertexBuffer(logicalDevice, physicalDevice, surface, commandPool, queue);
		indexBuffer = new IndexBuffer(logicalDevice, physicalDevice, surface, commandPool, queue);
		uniformBuffer = new UniformBuffer(logicalDevice, physicalDevice, surface, commandPool, queue);
		vertexBuffer->vertices =
		{
			{ { -0.5f,-1.5f },{ 1.0f,1.0f,1.0f } },
		{ { 0.5f,-0.5f },{ 1.0f,0.0f,0.0f } },
		{ { 0.5f,0.5f },{ 0.0f,1.0f,0.0f } },
		{ { -0.5f,0.5f },{ 0.0f,0.0f,1.0f } }
		};
		this->logicalDevice = logicalDevice;
	}


	Gameobject::~Gameobject()
	{
		vertexBuffer->CleanUp(logicalDevice);
		indexBuffer->CleanUp(logicalDevice);
		uniformBuffer->CleanUp(logicalDevice);
	}

}