#include "RenderableObject.h"



namespace RetuEngine
{
	RenderableObject::RenderableObject(RenderInterface* renderer, Model* model, Texture* texture)
	{
		this->indexBuffer = model->GetIndexBuffer();
		this->vertexBuffer = model->GetVertexBuffer();
		this->uniformBuffer = new UniformBuffer(renderer);
		//this->uniformBuffer = model->GetUniformBuffer();

		this->model = model;
		this->texture = texture;
		this->renderer = renderer;
	}

	void RenderableObject::CleanUp()
	{
		uniformBuffer->CleanUpBuffer();
	}

	/*RSS System*/

	RenderableSaveSystem::RenderableSaveSystem(const char* path)
	{
		file = nullptr;
		this->path = path;
		uint16_t saveDataSize = 0;

		fopen_s(&file, path, "rb");
		if (file == nullptr) 
		{ 
			std::cout << "Failed to open renderableobject save file" << std::endl; 
			fopen_s(&file, path, "wb");
			fwrite(&saveDataSize, sizeof(uint16_t), 1, file);
			fclose(file);
			return;
		}

		fread(&saveDataSize, sizeof(uint16_t), 1, file);
		saveData = std::vector<SaveStructure>(saveDataSize);


		for (int i = 0; i < saveDataSize; i++)
		{
			SaveStructure temp;
			uint16_t sizeOfName = 0;
			uint16_t sizeOfModel = 0;
			uint16_t sizeOfTexture = 0;

			fread(&temp.transformation, sizeof(glm::mat4), 1, file);
			fread(&sizeOfName, sizeof(uint16_t), 1, file);
			fread(&sizeOfModel, sizeof(uint16_t), 1, file);
			fread(&sizeOfTexture, sizeof(uint16_t), 1, file);

			temp.name = std::string(sizeOfName, '\0');
			temp.model = std::string(sizeOfModel, '\0');
			temp.texture = std::string(sizeOfTexture, '\0');

			fread(&temp.name[0], sizeof(char),sizeOfName, file);
			fread(&temp.model[0], sizeof(char),sizeOfModel, file);
			fread(&temp.texture[0], sizeof(char), sizeOfTexture, file);
			saveData[i] = temp;
		}

		fclose(file);
	}

	void RenderableSaveSystem::LoadAll()
	{
		file = nullptr;
		uint16_t saveDataSize = 0;

		fopen_s(&file, path.c_str(), "rb");
		if (file == nullptr) 
		{ 
			std::cout << "Failed to open renderableobject save file" << std::endl; 
			fopen_s(&file, path.c_str(), "wb");
			fwrite(&saveDataSize, sizeof(uint16_t), 1, file);
			fclose(file);
			return;
		}

		fread(&saveDataSize, sizeof(uint16_t), 1, file);
		saveData = std::vector<SaveStructure>(saveDataSize);


		for (int i = 0; i < saveDataSize; i++)
		{
			SaveStructure temp;
			uint16_t sizeOfName = 0;
			uint16_t sizeOfModel = 0;
			uint16_t sizeOfTexture = 0;

			fread(&temp.transformation, sizeof(glm::mat4), 1, file);

			fread(&sizeOfName, sizeof(uint16_t), 1, file);
			fread(&sizeOfModel, sizeof(uint16_t), 1, file);
			fread(&sizeOfTexture, sizeof(uint16_t), 1, file);

			temp.name = std::string(sizeOfName, '\0');
			temp.model = std::string(sizeOfModel, '\0');
			temp.texture = std::string(sizeOfTexture, '\0');

			fread(&temp.name[0], sizeof(char),sizeOfName, file);
			fread(&temp.model[0], sizeof(char),sizeOfModel, file);
			fread(&temp.texture[0], sizeof(char), sizeOfTexture, file);
			saveData[i] = temp;
		}

		fclose(file);
	}

	

	void RenderableSaveSystem::StopAndSave()
	{
		fopen_s(&file, path.c_str(), "wb");
		if (file == nullptr) { std::cout << "Failed to save renderableobject data" << std::endl; return; }
		uint16_t saveDataSize = saveData.size();

		fwrite(&saveDataSize, sizeof(uint16_t), 1, file);

		for (SaveStructure var : saveData)
		{
			fwrite(&var.transformation, sizeof(glm::mat4), 1, file);

			uint16_t sizeOfName = var.name.size();
			uint16_t sizeOfModel = var.model.size();
			uint16_t sizeOfTexture = var.texture.size();

			fwrite(&sizeOfName, sizeof(uint16_t), 1, file);
			fwrite(&sizeOfModel, sizeof(uint16_t), 1, file);
			fwrite(&sizeOfTexture, sizeof(uint16_t), 1, file);

			fwrite(&var.name[0], sizeof(char), sizeOfModel, file);
			fwrite(&var.model[0], sizeof(char), sizeOfModel, file);
			fwrite(&var.texture[0], sizeof(char), sizeOfTexture, file);
		}
		fclose(file);
	}

	void RenderableSaveSystem::AppendToSaveFile(const char* name, RenderableObject* object)
	{
		this->saveData.emplace_back(name,object->Transform, object->model->name, object->texture->name);
	}

	void RenderableSaveSystem::SaveAll(RenderableVector listOfObjects)
	{
		fopen_s(&file, path.c_str(), "wb");
		if (file == nullptr) { std::cout << "Failed to save renderableobject data" << std::endl; return; }

		uint16_t saveDataSize = listOfObjects.size();
		fwrite(&saveDataSize, sizeof(uint16_t), 1, file);

		for (int i = 0; i < listOfObjects.size(); i++)
		{
			RenderableObject* object = listOfObjects[i];
			SaveStructure var;

			var.transformation = object->Transform;
			var.name = listOfObjects.GetName(i);
			var.model = object->model->name;
			var.texture = object->texture->name;

			fwrite(&var.transformation, sizeof(glm::mat4), 1, file);

			uint16_t sizeOfName = var.name.size();
			uint16_t sizeOfModel = var.model.size();
			uint16_t sizeOfTexture = var.texture.size();

			fwrite(&sizeOfName, sizeof(uint16_t), 1, file);
			fwrite(&sizeOfModel, sizeof(uint16_t), 1, file);
			fwrite(&sizeOfTexture, sizeof(uint16_t), 1, file);

			fwrite(&var.name[0], sizeof(char), sizeOfName, file);
			fwrite(&var.model[0], sizeof(char), sizeOfModel, file);
			fwrite(&var.texture[0], sizeof(char), sizeOfTexture, file);
		}
		fclose(file);
	}
}