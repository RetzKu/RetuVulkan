#pragma once
#include <vulkan\vulkan.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"

namespace RetuEngine
{
	class RenderableObject
	{
	public:
		RenderableObject(RenderInterface* renderer,Camera* camera);
		RenderableObject(RenderInterface* renderer,Camera* camera,std::vector<Vertex> vertices);

		void CleanUp(VkDevice* device);
		VkBuffer* GetVertexBuffer() { return vertexBuffer->GetBuffer(); }
		VkBuffer* GetIndexBuffer() { return indexBuffer->GetBuffer(); }
		VkBuffer* GetUniformBuffer() { return uniformBuffer->GetBuffer(); }

		uint32_t GetIndicesSize() { return indexBuffer->GetIndicesSize(); }
		VkDeviceSize GetUniformBufferSize() { return uniformBuffer->GetUniformBufferSize(); }
		void UpdateUniform(VkDevice* logicalDevice, VkExtent2D extent) { uniformBuffer->Update(logicalDevice, extent); }

	/*Private Functions*/
	private:
		bool CreateVertexBuffer(RenderInterface* renderer);
		bool CreateVertexBuffer(RenderInterface* renderer,std::vector<Vertex> vertices);
		bool CreateUniformBuffer(RenderInterface* renderer, Camera* camera);
		bool CreateIndexBuffer(RenderInterface* renderer);

	/*Private Variables*/
	private:
		VertexBuffer* vertexBuffer;
		IndexBuffer* indexBuffer;
		UniformBuffer* uniformBuffer;
	};

}
