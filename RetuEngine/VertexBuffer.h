#pragma once
#include "Vertex.h"
#include <vector>
#include "Buffer.h"
#include "RenderInterface.h"

namespace RetuEngine
{
	class VertexBuffer : public Buffer
	{
	public:
		VertexBuffer(RenderInterface* renderer);
		VertexBuffer(RenderInterface* renderer, std::vector<Vertex> customVertices);
		~VertexBuffer();

		/*Getters*/
		uint32_t GetVerticesSize() { return vertices.size(); }

		void Create(RenderInterface* renderer);
		void CleanUp(const VkDevice* logicalDevice);
		//void AddToBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue, std::vector<Vertex> newVertices);

		std::vector<Vertex> vertices = 
		{
			{ { -0.5f, -0.5f,0.0f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
			{ { 0.5f, -0.5f,0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
			{ { 0.5f, 0.5f,0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
			{ { -0.5f, 0.5f,0.0f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } }
		};
	private:
	};
}
