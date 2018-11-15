#pragma once
#include <vulkan\vulkan.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"
#include "Texture.h"

namespace RetuEngine
{
	enum ObjectType
	{
		OBJECT_TYPE_SPRITE,
		OBJECT_TYPE_MODEL,
	};

	class RenderableObject
	{
	public:
		RenderableObject() {};
		//RenderableObject(RenderInterface* renderer,Camera* camera,const char* file);
		//RenderableObject(RenderInterface* renderer,Camera* camera,std::vector<Vertex> vertices, const char* file);

		void CleanUp(VkDevice* device);
		VkBuffer* GetVertexBuffer() { return vertexBuffer->GetBuffer(); }
		VkBuffer* GetIndexBuffer() { return indexBuffer->GetBuffer(); }
		VkBuffer* GetUniformBuffer() { return uniformBuffer->GetBuffer(); }
		VkDescriptorSet* GetDescriptorSet() { return &descriptorSet; }

		uint32_t GetIndicesSize() { return indexBuffer->GetIndicesSize(); }
		VkDeviceSize GetUniformBufferSize() { return sizeof(glm::mat4); }
		void UpdateUniform() { uniformBuffer->Update(); }

		Texture* texture;

		bool CreateVertexBuffer(RenderInterface* renderer);
		bool CreateVertexBuffer(RenderInterface* renderer,std::vector<Vertex> vertices);
		bool CreateUniformBuffer(RenderInterface* renderer);
		bool CreateIndexBuffer(RenderInterface* renderer);
		bool CreateIndexBuffer(RenderInterface* renderer, std::vector<uint32_t> indices);

	/*Private Variables*/
		ObjectType objectType;
		VertexBuffer* vertexBuffer;
		IndexBuffer* indexBuffer;
		UniformBuffer* uniformBuffer;
		VkDescriptorSet descriptorSet;
	};

}
