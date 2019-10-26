#define TINYOBJLOADER_IMPLEMENTATION
#include "Model.h"
#include <unordered_map>
#include <functional>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
	template<> struct hash<Engine::Vertex> 
	{
		size_t operator()(Engine::Vertex const& vertex) const 
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
					(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
					(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

namespace Engine
{

	Model::Model(RenderInterface* renderer, const char* filepath)
	{
		LoadModel(filepath);

		vertexBuffer = new VertexBuffer(renderer, vertices);
		indexBuffer = new IndexBuffer(renderer, indices);
	}

	Model::Model(RenderInterface* renderer, std::vector<Vertex> newVertices, std::vector<uint32_t> indices)
	{
		vertexBuffer = new VertexBuffer(renderer, newVertices);
		indexBuffer = new IndexBuffer(renderer, indices);
	}

	Model::Model(RenderInterface* renderer, float newVertices[], uint32_t size, std::vector<uint32_t> indices)
	{
		vertexBuffer = new VertexBuffer(renderer, newVertices, size);
		indexBuffer = new IndexBuffer(renderer, indices);
	}

	void Model::Delete()
	{
		vertexBuffer->CleanUpBuffer();
		indexBuffer->CleanUpBuffer();
	}

	//Model::Model(RenderInterface * renderer, const char * filepath, Texture* texture) : RenderableObject(renderer)
	//{
	//	LoadModel(filepath);

	//	this->texture = texture;
	//	CreateVertexBuffer(vertices);
	//	CreateIndexBuffer(indices);
	//	CreateUniformBuffer();
	//}

	//Model::Model(RenderInterface * renderer, const char * filepath, glm::vec3 color) : RenderableObject(renderer)
	//{
	//	LoadModel(filepath);

	//	this->color = glm::vec4(color,1);
	//	this->texture = nullptr;

	//	CreateVertexBuffer(vertices);
	//	CreateIndexBuffer(indices);
	//	CreateUniformBuffer();
	//}

	void ReadFilePath(const char* filepath, std::string* result)
	{
		std::string search = filepath;

		for (char character : search)
		{
			if (character == '.')
			{
				break;
			}
			*result += character;
		}
		*result += ".rm";
	}

	glm::vec3 calculateNormal(glm::vec3 a, glm::vec3 b, glm::vec3 c)
	{
		glm::vec3 first = b - a;
		glm::vec3 second = c - a;
		glm::vec3 normal = glm::normalize(glm::cross(first, second));
		return normal;
	}

	void Model::LoadModel(const char* filepath)
	{
		std::string filename;

		ReadFilePath(filepath,&filename);

		FILE* file;
		fopen_s(&file, filename.c_str(), "rb");

		if (file != nullptr)
		{
			uint32_t verticesSize = 0;
			uint32_t indicesSize = 0;
			fread(&verticesSize, sizeof(uint32_t), 1, file);
			fread(&indicesSize, sizeof(uint32_t), 1, file);

			vertices = std::vector<Vertex>(verticesSize);
			fread(&vertices[0], sizeof(Vertex), verticesSize, file);
			indices = std::vector<uint32_t>(indicesSize);
			fread(&indices[0], sizeof(uint32_t), indicesSize, file);
			fclose(file);
			return;
		}

		if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &err, filepath))
		{
			std::cout << "Failed to open Model: " << filepath << std::endl;
			return;
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		bool hasNormals = true;

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};

				vertex.pos = {
					attributes.vertices[3 * index.vertex_index + 0],
					attributes.vertices[3 * index.vertex_index + 1],
					attributes.vertices[3 * index.vertex_index + 2]
				};

				if (attributes.normals.size() == 0)
				{
					hasNormals = false;
				}

				if (hasNormals)
				{
					vertex.normal =
					{
						attributes.normals[3 * index.normal_index + 0],
						attributes.normals[3 * index.normal_index + 1],
						attributes.normals[3 * index.normal_index + 2]
					};
				}

				vertex.texCoord = {
					attributes.texcoords[2 * index.texcoord_index + 0],
					1.0f - attributes.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = { 1,1,1 };

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
		if (!hasNormals)
		{
			std::cout << "Calculating normals";
			for (int i = 0; i < indices.size(); i += 3)
			{
				glm::vec3 a = vertices[indices[i]].pos;
				glm::vec3 b = vertices[indices[i+1]].pos;
				glm::vec3 c = vertices[indices[i+2]].pos;
				glm::vec3 result = calculateNormal(a, b, c);
				vertices[indices[i]].normal = result;
				vertices[indices[i + 1]].normal = result;
				vertices[indices[i + 2]].normal = result;
			}
		}

		fopen_s(&file,filename.c_str(), "wb");
		size_t sizeOfVector = vertices.size();
		size_t sizeOfIndices = indices.size();
		fwrite(&sizeOfVector, sizeof(uint32_t), 1, file);
		fwrite(&sizeOfIndices, sizeof(uint32_t), 1, file);
		fwrite(vertices.data(), sizeof(Vertex), sizeOfVector,file);
		fwrite(indices.data(), sizeof(uint32_t), sizeOfIndices, file);
		fclose(file);
	}
}