#include "VertexBuffer.h"
#include <iostream>

namespace RetuEngine
{
	void VertexBuffer::Create()
	{
		bufferSize = sizeof(Vertex)* vertices.size();

		StartMapping(bufferSize,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		Map(vertices.data(), bufferSize);
		StopMapping();
	}
}