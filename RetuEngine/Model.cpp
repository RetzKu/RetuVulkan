#define TINYOBJLOADER_IMPLEMENTATION
#include "Model.h"
#include <unordered_map>
#include <functional>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
	template<> struct hash<RetuEngine::Vertex> 
	{
		size_t operator()(RetuEngine::Vertex const& vertex) const 
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
					(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
					(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

namespace RetuEngine
{
	Model::Model(RenderInterface * renderer,Camera* camera, const char * filepath, glm::vec3 offset)
	{
		this->renderer = renderer;
		LoadModel(filepath);
		for(int i = 0; i < vertices.size(); i++)
		{
			vertices[i].pos += offset;
		}
		CreateVertexBuffer(renderer,vertices);
		CreateIndexBuffer(renderer,indices);
		CreateUniformBuffer(renderer, camera);
	}

	Model::Model(RenderInterface * renderer,Camera* camera, const char * filepath, Texture* texture)
	{
		this->renderer = renderer;
		LoadModel(filepath);
		this->texture = texture;
		CreateVertexBuffer(renderer,vertices);
		CreateIndexBuffer(renderer,indices);
		CreateUniformBuffer(renderer, camera);
	}
	
	Model::~Model()
	{

	}

	void Model::LoadModel(const char* filepath)
	{
		if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &err, filepath))
		{
			throw std::runtime_error("Failed to load model");
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};

				vertex.pos = {
					attributes.vertices[3 * index.vertex_index + 0],
					attributes.vertices[3 * index.vertex_index + 1],
					attributes.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attributes.texcoords[2 * index.texcoord_index + 0],
					1.0f - attributes.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}
}