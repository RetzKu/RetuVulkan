#include "RenderableObject.h"



namespace RetuEngine
{
	RenderableObject::RenderableObject(RenderInterface* renderer)
	{
		CreateVertexBuffer(renderer);
		CreateIndexBuffer(renderer);
	}


	RenderableObject::~RenderableObject()
	{
	}

	bool RenderableObject::CreateIndexBuffer(RenderInterface* renderer)
	{
		indexBuffer = new IndexBuffer(renderer);
		if (indexBuffer != nullptr) { throw std::runtime_error("Failed to create Index Buffer"); return true; }
		else { return false; }
	}

	bool RenderableObject::CreateVertexBuffer(RenderInterface* renderer)
	{
		vertexBuffer = new VertexBuffer(renderer);
		if (vertexBuffer != nullptr) { throw std::runtime_error("Failed to create Vertex Buffer"); return true; }
		else { return false; }
	}
}