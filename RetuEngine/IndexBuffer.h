#pragma once
#include "Buffer.h"
#include "RenderInterface.h"

namespace Engine 
{
	class IndexBuffer : public Buffer
	{
	public:
		IndexBuffer(RenderInterface* renderer);
		IndexBuffer(RenderInterface* renderer,std::vector<uint32_t> indices);

		void Create(RenderInterface* renderer);

		uint32_t GetIndicesSize() { return (uint32_t)indices.size(); }
	protected:
		//std::vector<uint32_t> indices =
		//{
		//	0,3,2,2,1,0
		//};
		std::vector<uint32_t> indices = {
		 0,  1,  2,  0,  2,  3,   //front
		 4,  5,  6,  4,  6,  7,   //right
		 8,  9,  10, 8,  10, 11,  //back
		 12, 13, 14, 12, 14, 15,  //left
		 16, 17, 18, 16, 18, 19,  //upper
		 20, 21, 22, 20, 22, 23 }; //bottomk
	};

}
