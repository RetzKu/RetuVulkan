#include "IndexBuffer.h"

namespace Engine 
{
	IndexBuffer::IndexBuffer(RenderInterface* renderer) : Buffer(renderer)
	{
		Create(renderer);
	}
	IndexBuffer::IndexBuffer(RenderInterface* renderer,std::vector<uint32_t> indices) : Buffer(renderer)
	{
		this->indices = indices;
		Create(renderer);
	}

	void IndexBuffer::Create(RenderInterface* renderer)
	{
		VkDeviceSize bufferSize = sizeof(indices[0])* indices.size();
		int size = sizeof(uint32_t);
		StartMapping(bufferSize,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		Map(indices.data(), (size_t)bufferSize);
		StopMapping();
	}
}