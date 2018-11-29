#pragma once
#include <stb_image.h>
#include <vector>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderInterface.h"

#include <tiny_obj_loader.h>

namespace RetuEngine
{
	enum DefaultModelType
	{
		CUBE,
		SPHERE,
		PLANE,
		TRIANGLE,
	};

	class Model 
	{
	public:
		Model(RenderInterface* renderer, const char* filepath);

		void Delete();

		IndexBuffer* GetIndexBuffer() { return indexBuffer; }
		VertexBuffer* GetVertexBuffer() { return vertexBuffer; }
		//UniformBuffer* GetUniformBuffer() { return uniformBuffer; }

		std::string name;
	private:
		void LoadModel(const char* filepath);

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		VertexBuffer* vertexBuffer;
		IndexBuffer* indexBuffer;
		//UniformBuffer* uniformBuffer;

		RenderInterface* renderer;

		tinyobj::attrib_t attributes;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
	};

	class ModelVector 
	{
	public:
		ModelVector() {};

		void CleanUp()
		{
			for (Model* var : models)
			{
				//var->CleanUp();
				var->Delete();
			}
			for (Model* var : models)
			{
				delete var;
			}
			models.clear();
		}

		void Push(const char* name, Model model)
		{
			bool taken = false;
			for (std::string var : names)
			{
				if(strcmp(var.c_str(),name) == 0)
				{
					std::cout << "Name taken: " << name << std::endl;
					taken = true;
					break;
				}
			}
			if (!taken)
			{
				model.name = name;
				models.push_back(new Model(model));
				names.push_back(name);
			}
		}

		Model* Get(std::string name)
		{
			int index = 0;
			for (std::string var : names)
			{
				if (strcmp(name.c_str(), var.c_str()) == 0)
				{
					return models[index];
				}
				index++;
			}
			return nullptr;
		}
		
	private:
		std::vector<Model*> models;
		std::vector<std::string> names;
		std::vector<int> freeSlots;
	};
}