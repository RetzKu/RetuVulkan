#pragma once
#include <vulkan\vulkan.h>
//#include "VertexBuffer.h"
//#include "IndexBuffer.h"
//#include "UniformBuffer.h"
//#include "Texture.h"
#include "RenderableObject.h"

namespace RetuEngine
{
	class Sprite : public RenderableObject
	{
	public:
		Sprite(RenderInterface* renderer, Camera* camera, const char* file);
		Sprite(RenderInterface* renderer, Camera* camera, std::vector<Vertex> vertices, const char* file);

		//void CleanUp(VkDevice* device);
		//VkBuffer* GetVertexBuffer() { return vertexBuffer->GetBuffer(); }
		//VkBuffer* GetIndexBuffer() { return indexBuffer->GetBuffer(); }
		//VkBuffer* GetUniformBuffer() { return uniformBuffer->GetBuffer(); }
		//VkDescriptorSet* GetDescriptorSet() { return &descriptorSet; }

		//uint32_t GetIndicesSize() { return indexBuffer->GetIndicesSize(); }
		//VkDeviceSize GetUniformBufferSize() { return uniformBuffer->GetUniformBufferSize(); }
		//void UpdateUniform(VkDevice* logicalDevice, VkExtent2D extent) { uniformBuffer->Update(logicalDevice, extent); }
	};
}
//namespace RetuEngine
//{
//	class Test : public RenderableObject
//	{
//		Test() { };
//	};
//}
