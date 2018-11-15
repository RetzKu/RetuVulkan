#include "RenderableObject.h"



namespace RetuEngine
{
	//RenderableObject::RenderableObject(RenderInterface* renderer,Camera* camera,const char* file)
	//{
	//	texture = new Texture(file, renderer);
	//	CreateVertexBuffer(renderer);
	//	CreateIndexBuffer(renderer);
	//	CreateUniformBuffer(renderer,camera);
	//}
	//RenderableObject::RenderableObject(RenderInterface* renderer, Camera* camera, std::vector<Vertex> vertices,const char* file)
	//{
	//	texture = new Texture(file, renderer);
	//	CreateVertexBuffer(renderer,vertices);
	//	CreateIndexBuffer(renderer);
	//	CreateUniformBuffer(renderer,camera);
	//}

	void RenderableObject::CleanUp(VkDevice* device)
	{
		vertexBuffer->CleanUp(device);
		indexBuffer->CleanUp(device);
		uniformBuffer->CleanUp();
	}

	bool RenderableObject::CreateIndexBuffer(RenderInterface* renderer)
	{
		indexBuffer = new IndexBuffer(renderer);
		if (indexBuffer == nullptr) { throw std::runtime_error("Failed to create Index Buffer"); return false; }
		else { return true; }
	}

	bool RenderableObject::CreateIndexBuffer(RenderInterface* renderer, std::vector<uint32_t> indices)
	{
		indexBuffer = new IndexBuffer(renderer, indices);
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
	bool RenderableObject::CreateUniformBuffer(RenderInterface * renderer)
	{
		uniformBuffer = new UniformBuffer(renderer);
		if (uniformBuffer == nullptr) { throw std::runtime_error("Failed to create Uniform Buffer"); return false; }
		else { return true; }
	}
}