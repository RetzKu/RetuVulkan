#include "VertexBuffer.h"
#include <iostream>

namespace Engine
{
	void VertexBuffer::Create()
	{
		bufferSize = sizeof(Vertex)* vertices.size();

		StartMapping(bufferSize,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		Map(vertices.data(), bufferSize);
		StopMapping();
	}

	void VertexBuffer::Create(float vertices[], uint32_t size)
	{
		bufferSize = sizeof(float) * size;
		this->size = size;
		for (int i = 0; i < 108; i++)
		{
			float vertti = vertices[i];
		}
		StartMapping(bufferSize,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		Map(vertices, bufferSize);
		StopMapping();
	}
}