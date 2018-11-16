#pragma once
#include <stb_image.h>
#include <vector>
#include "VertexBuffer.h"
#include "RenderInterface.h"
#include <tiny_obj_loader.h>
#include "RenderableObject.h"

namespace RetuEngine
{
	enum DefaultModelType
	{
		CUBE,
		SPHERE,
		PLANE,
		TRIANGLE,
	};

	class Model : public RenderableObject 
	{
	public:
		Model(RenderInterface* renderer, const char* filepath);
		Model(RenderInterface * renderer, const char * filepath,Texture* texture);
		Model(RenderInterface * renderer, const char * filepath,glm::vec3 color);

		void LoadModel(const char* filepath);

	private:
		int height;
		int width;
		int channels;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		VertexBuffer* vertexBuffer;

		RenderInterface* renderer;

		glm::vec3 position;
		glm::vec4 color;

		tinyobj::attrib_t attributes;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
	};

}