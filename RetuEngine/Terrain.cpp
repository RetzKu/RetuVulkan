#include "Terrain.h"

namespace Engine
{
	Terrain::Terrain(RenderInterface * renderer, Model * model, std::vector<Texture*> textures)
	{
		this->renderer = renderer;
		this->model = model;
		this->textures = textures;

		this->indexBuffer = model->GetIndexBuffer();
		this->vertexBuffer = model->GetVertexBuffer();
		this->uniformBuffer = new UniformBuffer(renderer);
	}

	void Terrain::CleanUp()
	{
		uniformBuffer->CleanUpBuffer();
	}
}