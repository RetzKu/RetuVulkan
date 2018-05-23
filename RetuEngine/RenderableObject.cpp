#include "RenderableObject.h"



namespace RetuEngine
{
	RenderableObject::RenderableObject(RenderInterface* renderer,Camera* camera)
	{
		CreateVertexBuffer(renderer);
		CreateIndexBuffer(renderer);
		CreateUniformBuffer(renderer,camera);
	}
	RenderableObject::RenderableObject(RenderInterface* renderer, Camera* camera, std::vector<Vertex> vertices)
	{
		CreateVertexBuffer(renderer,vertices);
		CreateIndexBuffer(renderer);
		CreateUniformBuffer(renderer,camera);
	}

	void RenderableObject::CleanUp(VkDevice* device)
	{
		vertexBuffer->CleanUp(device);
		indexBuffer->CleanUp(device);
		uniformBuffer->CleanUp(device);
	}

	bool RenderableObject::CreateIndexBuffer(RenderInterface* renderer)
	{
		indexBuffer = new IndexBuffer(renderer);
		if (indexBuffer == nullptr) { throw std::runtime_error("Failed to create Index Buffer"); return false; }
		else { return true; }
	}

	bool RenderableObject::CreateVertexBuffer(RenderInterface* renderer,std::vector<Vertex> vertices)
	{
		vertexBuffer = new VertexBuffer(renderer,vertices);
		if (vertexBuffer == nullptr) { throw std::runtime_error("Failed to create Vertex Buffer"); return false; }
		else { return true; }
	}

	bool RenderableObject::CreateVertexBuffer(RenderInterface* renderer)
	{
		vertexBuffer = new VertexBuffer(renderer);
		if (vertexBuffer == nullptr) { throw std::runtime_error("Failed to create Vertex Buffer"); return false; }
		else { return true; }
	}
	bool RenderableObject::CreateUniformBuffer(RenderInterface * renderer, Camera * camera)
	{
		uniformBuffer = new UniformBuffer(renderer, camera);
		if (uniformBuffer == nullptr) { throw std::runtime_error("Failed to create Uniform Buffer"); return false; }
		else { return true; }
	}
}