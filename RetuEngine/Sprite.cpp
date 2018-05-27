#include "Sprite.h"



namespace RetuEngine
{
	Sprite::Sprite(RenderInterface* renderer, Camera* camera, const char* file)
	{
		texture = new Texture(file, renderer);
		objectType = OBJECT_TYPE_SPRITE;
		CreateVertexBuffer(renderer);
		CreateIndexBuffer(renderer);
		CreateUniformBuffer(renderer, camera);
	}
	Sprite::Sprite(RenderInterface* renderer, Camera* camera, std::vector<Vertex> vertices, const char* file)
	{
		texture = new Texture(file, renderer);
		objectType = OBJECT_TYPE_SPRITE;
		CreateVertexBuffer(renderer, vertices);
		CreateIndexBuffer(renderer);
		CreateUniformBuffer(renderer, camera);
	}
}