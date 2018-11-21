#include "RenderableObject.h"



namespace RetuEngine
{
	RenderableObject::RenderableObject(RenderInterface* renderer, Model* model, Texture* texture)
	{
		this->indexBuffer = model->GetIndexBuffer();
		this->vertexBuffer = model->GetVertexBuffer();
		this->uniformBuffer = new UniformBuffer(renderer);
		//this->uniformBuffer = model->GetUniformBuffer();

		this->model = model;
		this->texture = texture;
		this->renderer = renderer;
	}

	void RenderableObject::CleanUp()
	{
		uniformBuffer->CleanUpBuffer();
	}

	//bool RenderableObject::CreateIndexBuffer()
	//{
	//	indexBuffer = new IndexBuffer(renderer);
	//	if (indexBuffer == nullptr) { throw std::runtime_error("Failed to create Index Buffer"); return false; }
	//	else { return true; }
	//}

	//bool RenderableObject::CreateIndexBuffer(std::vector<uint32_t> indices)
	//{
	//	indexBuffer = new IndexBuffer(renderer, indices);
	//	if (indexBuffer == nullptr) { throw std::runtime_error("Failed to create Index Buffer"); return false; }
	//	else { return true; }
	//}

	//bool RenderableObject::CreateVertexBuffer(std::vector<Vertex> vertices)
	//{
	//	vertexBuffer = new VertexBuffer(renderer,vertices);
	//	if (vertexBuffer == nullptr) { throw std::runtime_error("Failed to create Vertex Buffer"); return false; }
	//	else { return true; }
	//}

	//bool RenderableObject::CreateVertexBuffer()
	//{
	//	vertexBuffer = new VertexBuffer(renderer);
	//	if (vertexBuffer == nullptr) { throw std::runtime_error("Failed to create Vertex Buffer"); return false; }
	//	else { return true; }
	//}
	//bool RenderableObject::CreateUniformBuffer()
	//{
	//	uniformBuffer = new UniformBuffer(renderer);
	//	if (uniformBuffer == nullptr) { throw std::runtime_error("Failed to create Uniform Buffer"); return false; }
	//	else { return true; }
	//}
}