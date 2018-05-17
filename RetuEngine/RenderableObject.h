#pragma once
#include <vulkan\vulkan.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace RetuEngine
{
	class RenderableObject
	{
	public:
		RenderableObject(RenderInterface* renderer);
		~RenderableObject();

	/*Private Functions*/
	private:
		bool CreateVertexBuffer(RenderInterface* renderer);
		bool CreateIndexBuffer(RenderInterface* renderer);

	/*Private Variables*/
	private:
		VertexBuffer* vertexBuffer;
		IndexBuffer* indexBuffer;
	};

}
