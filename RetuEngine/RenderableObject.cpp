#include "RenderableObject.h"



namespace RetuEngine
{
	void RenderableObject::CleanUp()
	{
		vertexBuffer->CleanUpBuffer();
		indexBuffer->CleanUpBuffer();
		uniformBuffer->CleanUpBuffer();
	}

	bool RenderableObject::CreateIndexBuffer()
	{
		indexBuffer = new IndexBuffer(renderer);
		if (indexBuffer == nullptr) { throw std::runtime_error("Failed to create Index Buffer"); return false; }
		else { return true; }
	}

	bool RenderableObject::CreateIndexBuffer(std::vector<uint32_t> indices)
	{
		indexBuffer = new IndexBuffer(renderer, indices);
		if (indexBuffer == nullptr) { throw std::runtime_error("Failed to create Index Buffer"); return false; }
		else { return true; }
	}

	bool RenderableObject::CreateVertexBuffer(std::vector<Vertex> vertices)
	{
		vertexBuffer = new VertexBuffer(renderer,vertices);
		if (vertexBuffer == nullptr) { throw std::runtime_error("Failed to create Vertex Buffer"); return false; }
		else { return true; }
	}

	bool RenderableObject::CreateVertexBuffer()
	{
		vertexBuffer = new VertexBuffer(renderer);
		if (vertexBuffer == nullptr) { throw std::runtime_error("Failed to create Vertex Buffer"); return false; }
		else { return true; }
	}
	bool RenderableObject::CreateUniformBuffer()
	{
		uniformBuffer = new UniformBuffer(renderer);
		if (uniformBuffer == nullptr) { throw std::runtime_error("Failed to create Uniform Buffer"); return false; }
		else { return true; }
	}
}