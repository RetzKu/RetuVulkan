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

		void Create(RenderInterface* renderer);

		uint32_t GetIndicesSize() { return indices.size(); }
	protected:
		std::vector<uint32_t> indices =
		{
			0,3,2,2,1,0
		};
	};

}
