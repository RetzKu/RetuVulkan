#pragma once
#include <vulkan\vulkan.h> 
#include "UniformBuffer.h"
#include "Texture.h"
#include "Model.h"

namespace Engine
{
	class Terrain
	{
	public:
		Terrain() {};
		Terrain(RenderInterface* renderer, Model* model, std::vector<Texture*> textures);

		void CleanUp();
		VkBuffer* GetVertexBuffer() { return vertexBuffer->GetBuffer(); }
		VkBuffer* GetIndexBuffer() { return indexBuffer->GetBuffer(); }
		VkBuffer* GetUniformBuffer() { return uniformBuffer->GetBuffer(); }
		VkDescriptorSet* GetDescriptorSet() { return &descriptorSet; }
		uint32_t GetIndicesSize() { return indexBuffer->GetIndicesSize(); }
		void UpdateUniform() { uniformBuffer->Update(transform); }

		glm::mat4 transform;
		Model* model;
		std::vector<Texture*> textures;

	private:
		VertexBuffer* vertexBuffer;
		IndexBuffer* indexBuffer;
		UniformBuffer* uniformBuffer;
		VkDescriptorSet descriptorSet;
		RenderInterface* renderer;
	};

	class TerrainVector
	{
	public:
		TerrainVector() {};
		
		
	private:
		std::vector<Terrain*> terrains;
	};
}
