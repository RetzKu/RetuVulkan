#pragma once
#include <vulkan\vulkan.h>
#include "RenderableObject.h"

namespace RetuEngine
{
	class Sprite : public RenderableObject
	{
	public:
		Sprite(RenderInterface* renderer, Camera* camera, Texture* texture);
		Sprite(RenderInterface* renderer, Camera* camera, std::vector<Vertex> vertices, Texture* texture);
	};
}