#pragma once
#include "Buffer.h"
#include "RenderInterface.h"

namespace RetuEngine
{
	class IndexBuffer : public Buffer
	{
	public:
		IndexBuffer(RenderInterface* renderer);
		IndexBuffer(RenderInterface* renderer,std::vector<uint32_t> indices);
		~IndexBuffer();

		void Create(RenderInterface* renderer);
		void CleanUp(const VkDevice* logicalDevice);

		uint32_t GetIndicesSize() { return indices.size(); }

		std::vector<VkDeviceSize> offsets;
	protected:
		std::vector<uint32_t> indices =
		{
			0,1,2,2,3,0
		};
	};

}
