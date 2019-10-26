#pragma once
#include "Vertex.h"
#include <vector>
#include "Buffer.h"
#include "RenderInterface.h"

namespace Engine
{
	class VertexBuffer : public Buffer
	{
	public:
		VertexBuffer(RenderInterface* renderer) : Buffer(renderer) { Create(); };
		VertexBuffer(RenderInterface* renderer, std::vector<Vertex> customVertices) : Buffer(renderer) { vertices = customVertices; Create(); };
		VertexBuffer(RenderInterface* renderer, float vertices[], uint32_t size) :Buffer(renderer) { Create(vertices, size); };

		/*Getters*/
		uint32_t GetVerticesSize() 
		{ 
			if (vertices.size() == 0) { return size; }
			return (uint32_t)vertices.size(); 
		}


	private:
		std::vector<Vertex> vertices = {};
		uint32_t size;

		void Create();
		void Create(float vertices[], uint32_t size);
	};
}
