#pragma once
#include <vulkan\vulkan.h>
#include "RenderableObject.h"

namespace RetuEngine
{
	class Sprite : public RenderableObject
	{
	public:
		Sprite(RenderInterface* renderer, Texture* texture);
		Sprite(RenderInterface* renderer, std::vector<Vertex> vertices, Texture* texture);

	private:
		std::vector<Vertex> vertices = 
		{
			{ { -1.0f, 0.0f,-1.0f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } ,{0.0f,1.0f,0.0f}},
			{ { 1.0f, 0.0f,-1.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } ,{0.0f,1.0f,0.0f}},
			{ { 1.0f, 0.0f,1.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f } ,{0.0f,1.0f,0.0f}},
			{ { -1.0f, 0.0f,1.0f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 1.0f } ,{0.0f,1.0f,0.0f}}
		};
	};
}