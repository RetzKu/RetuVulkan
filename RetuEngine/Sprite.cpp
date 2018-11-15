#include "Sprite.h"



namespace RetuEngine
{
	Sprite::Sprite(RenderInterface* renderer, Camera* camera, Texture* texture)
	{
		//TODO: kehitä object type toimivaksi
		objectType = OBJECT_TYPE_SPRITE;
		this->texture = texture;

		CreateVertexBuffer(renderer);
		CreateIndexBuffer(renderer);
		CreateUniformBuffer(renderer);
	}
	Sprite::Sprite(RenderInterface* renderer, Camera* camera, std::vector<Vertex> vertices, Texture* texture)
	{
		objectType = OBJECT_TYPE_SPRITE;
		this->texture = texture;

		CreateVertexBuffer(renderer, vertices);
		CreateIndexBuffer(renderer);
		CreateUniformBuffer(renderer);
	}
}