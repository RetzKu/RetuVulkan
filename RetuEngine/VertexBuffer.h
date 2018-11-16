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
		VertexBuffer(RenderInterface* renderer) : Buffer(renderer) { Create(); };
		VertexBuffer(RenderInterface* renderer, std::vector<Vertex> customVertices) : Buffer(renderer) { vertices = customVertices; Create(); };

		/*Getters*/
		uint32_t GetVerticesSize() { return vertices.size(); }


	private:
		std::vector<Vertex> vertices = {};

		void Create();
	};
}
