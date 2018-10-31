#pragma once
#include <stb_image.h>
#include <vector>
#include "VertexBuffer.h"
#include "RenderInterface.h"
#include <tiny_obj_loader.h>
#include "RenderableObject.h"

namespace RetuEngine
{
	class Model : public RenderableObject 
	{
	public:
		Model(RenderInterface * renderer,Camera* camera, const char * filepath, glm::vec3 offset);
		Model(RenderInterface * renderer,Camera* camera, const char * filepath,Texture* texture);
		~Model();

		void LoadModel(const char* filepath);

	private:
		int height;
		int width;
		int channels;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		VertexBuffer* vertexBuffer;

		RenderInterface* renderer;

		tinyobj::attrib_t attributes;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
	};

}