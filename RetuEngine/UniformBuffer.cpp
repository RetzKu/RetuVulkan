#include "UniformBuffer.h"

namespace Engine 
{
	void UniformBuffer::Create()
	{
		//CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);

		StartMapping(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		Map(&modelMatrix, sizeof(glm::mat4));
		StopMapping();
	}

	void UniformBuffer::Update(glm::mat4 objectTransform)
	{
		glm::mat4 temp = modelMatrix * objectTransform;
		StartUpdate();
		UpdateMap(&temp, sizeof(glm::mat4));
		StopUpdate(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	}
}