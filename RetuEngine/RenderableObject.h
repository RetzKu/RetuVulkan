#pragma once
#include <vulkan\vulkan.h>

#include "UniformBuffer.h"
#include "Texture.h"
#include "Model.h"

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
		RenderableObject(RenderInterface* renderer) : renderer(renderer){};
		RenderableObject() {};
		RenderableObject(RenderInterface* renderer, Model* model, Texture* texture);
		//RenderableObject(RenderInterface* renderer,Camera* camera,const char* file);
		//RenderableObject(RenderInterface* renderer,Camera* camera,std::vector<Vertex> vertices, const char* file);

		void CleanUp();
		VkBuffer* GetVertexBuffer() { return vertexBuffer->GetBuffer(); }
		VkBuffer* GetIndexBuffer() { return indexBuffer->GetBuffer(); }
		VkBuffer* GetUniformBuffer() { return uniformBuffer->GetBuffer(); }
		VkDescriptorSet* GetDescriptorSet() { return &descriptorSet; }
		uint32_t GetIndicesSize() { return indexBuffer->GetIndicesSize(); }

		void UpdateUniform() { uniformBuffer->Update(Transform); }

		Model* model;
		Texture* texture;

		glm::mat4 Transform;

		//bool CreateVertexBuffer();
		//bool CreateVertexBuffer(std::vector<Vertex> vertices);
		//bool CreateUniformBuffer();
		//bool CreateIndexBuffer();
		//bool CreateIndexBuffer(std::vector<uint32_t> indices);

	/*Private Variables*/

		ObjectType objectType;
		VertexBuffer* vertexBuffer;
		IndexBuffer* indexBuffer;
		UniformBuffer* uniformBuffer;
		VkDescriptorSet descriptorSet;
		RenderInterface* renderer;
	};

	class RenderableVector 
	{
	public:
		RenderableVector() {};

		void CleanUp()
		{
			for (RenderableObject* var : models)
			{
				var->CleanUp();
			}
			for (RenderableObject* var : models)
			{
				delete var;
			}
			models.clear();
		}

		void Push(const char* name, RenderableObject model)
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
				models.push_back(new RenderableObject(model));
				names.push_back(name);
			}
		}

		RenderableObject* Get(int indx)
		{
			return models[indx];
		}

		std::string GetName(int index)
		{
			return names[index];
		}

		RenderableObject* Get(std::string name)
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

		RenderableObject* operator[](int index) { return models[index]; }
		uint16_t size() { return models.size(); }
		
	private:
		std::vector<RenderableObject*> models;
		std::vector<std::string> names;
		std::vector<int> freeSlots;
	};

	struct SaveStructure
	{
		glm::mat4 transformation;
		std::string model;
		std::string texture;
	};

	class RenderableSaveSystem
	{
	public:
		RenderableSaveSystem(const char* path);
		RenderableSaveSystem() {};

		void StopAndSave();
		void AppendToSaveFile(const char* name, RenderableObject* object);
		void LoadAll();
		void SaveAll(RenderableVector listOfObjects);

	private:
		std::vector<SaveStructure> saveData;
		std::string path;
		FILE* file;
	};
}
