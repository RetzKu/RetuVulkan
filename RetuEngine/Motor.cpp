#include "Motor.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include <map>
#include <algorithm>
#include <set>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm\gtc\random.hpp>

namespace RetuEngine
{

#define MAX_LIGHT_COUNT 200

	struct _Dummy_VisibleLightsForTile
	{
		uint32_t count;
		std::array<uint32_t, 63> lightindices;
	};

	Motor::~Motor()
	{
		delete renderer;
		delete swapChain;
		delete camera;
		delete inputManager;
		delete defaultTexture;
		models.CleanUp();
		renderables.CleanUp();
		textures.CleanUp();
	}




	
	void Motor::InitWindow()
	{
		windowObj.Create();
		glfwSetWindowUserPointer(windowObj.window, this);
		//glfwSetWindowSizeCallback(windowObj.window, Motor::OnWindowResized);
		glfwSetInputMode(windowObj.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		camera = new Camera();
		//glfwSetCursorPosCallback(windowObj.window, mouse_callback);
		inputManager = new Input(windowObj.window,windowObj.height, windowObj.width,camera);
	}

	void Motor::LoadTextures()
	{
		defaultTexture = new Texture("default.png", renderer);
		//textures.Push("hill", Texture("MapText.jpg", renderer));
		textures.Push("chalet", Texture("chalet.jpg",renderer));
		textures.Push("weeb", Texture("Weeb.bmp",renderer));
		textures.Push("grass", Texture("grasstex.jpg",renderer));
		textures.Push("cube-uv", Texture("CubeUV.png",renderer,true));
		textures.Push("sand", Texture("dirt.png", renderer));
		textures.Push("dirt", Texture("dirt.png", renderer));
		textures.Push("rock", Texture("rock.png", renderer));
		textures.Push("valley-blend", Texture("valley-blend.png", renderer));
	}

	void Motor::LoadModels()
	{
		models.Push("cube"	, Model(renderer, "Cube.obj"));
		models.Push("hill"	, Model(renderer, "Hill.obj"));
		models.Push("chalet", Model(renderer, "chalet.obj"));
		models.Push("bunny"	, Model(renderer, "Bunny.obj"));
		models.Push("valley", Model(renderer, "Valley.obj"));
	}








	void Motor::CreateLights()
	{
		for (int i = 0; i < 1; i++) {
			// do { color = { glm::linearRand(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)) }; } while (color.length() < 0.8f);
			
			pointLights.emplace_back(glm::linearRand(glm::vec3(1,1,0), glm::vec3(1,1,0)), 0.1f, 0.001f,glm::vec4(1,1,1,1));
		}

		int lightnum = pointLights.size();

		VkDeviceSize size = sizeof(Pointlight) * lightnum + sizeof(glm::vec4);

		pointLightBuffer = Buffer(renderer);
		pointLightBuffer.StartMapping(size,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pointLightBuffer.Map(&lightnum, sizeof(glm::vec4));
		pointLightBuffer.Map(pointLights.data(), sizeof(Pointlight)*pointLights.size());
		pointLightBuffer.StopMapping();

	}


	void Motor::CreateLightDescriptorSets()
	{
		VkDescriptorSetLayout layouts[] = { lightDescriptorSetlayout };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		if (vkAllocateDescriptorSets(renderer->logicalDevice, &allocInfo, &lightDescriptor) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create light descriptor set");
		}

		VkDescriptorBufferInfo pointLightBufferInfo = {};
		pointLightBufferInfo.buffer = *pointLightBuffer.GetBuffer();
		pointLightBufferInfo.offset = 0;
		pointLightBufferInfo.range = pointLightBuffer.bufferSize;

		std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = lightDescriptor;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &pointLightBufferInfo;

		vkUpdateDescriptorSets(renderer->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void Motor::CreateCameraDescriptorSets()
	{

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &cameraSetLayout;

		VkResult result = vkAllocateDescriptorSets(renderer->logicalDevice, &allocInfo, &cameraSet);

		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptorset");
		}
		else
		{
			std::cout << "Allocated descriptor succesfully" << std::endl;
		}

		//for cameraposition at frag
		cameraBuffer = Buffer(renderer);
		cameraBuffer.StartMapping(sizeof(glm::vec3),VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		cameraBuffer.Map(&camera->cameraPos, sizeof(glm::vec3));
		cameraBuffer.StopMapping();

		VkDescriptorBufferInfo cameraBufferInfo = {};
		cameraBufferInfo.buffer = *cameraBuffer.GetBuffer();
		cameraBufferInfo.offset = 0;
		cameraBufferInfo.range = cameraBuffer.bufferSize;

		camera->view = glm::lookAt(camera->cameraPos,  camera->cameraPos + camera->cameraFront, camera->cameraUp);
		camera->proj = glm::perspective(glm::radians(90.0f), swapChain->GetExtent()->width / (float)swapChain->GetExtent()->height, 0.1f, 10000.0f);
		camera->proj[1][1] *= -1; //flipping y coordinate?

		//for cameraViewMatrix at vert
		cameraViewBuffer = Buffer(renderer);
		cameraViewBuffer.StartMapping(sizeof(glm::mat4) * 2,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		cameraViewBuffer.Map(&camera->view,sizeof(glm::mat4));
		cameraViewBuffer.Map(&camera->proj,sizeof(glm::mat4));
		cameraViewBuffer.StopMapping();

		VkDescriptorBufferInfo cameraViewBufferInfo = {};
		cameraViewBufferInfo.buffer = *cameraViewBuffer.GetBuffer();
		cameraViewBufferInfo.offset = 0;
		cameraViewBufferInfo.range = cameraViewBuffer.bufferSize;

		std::vector<VkWriteDescriptorSet> camerawrites;
		VkWriteDescriptorSet cameraWriteSet = {};
		cameraWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraWriteSet.dstSet = cameraSet;
		cameraWriteSet.dstBinding = 0;
		cameraWriteSet.dstArrayElement = 0;
		cameraWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		cameraWriteSet.descriptorCount = 1;
		cameraWriteSet.pBufferInfo = &cameraBufferInfo;
		camerawrites.push_back(cameraWriteSet);

		VkWriteDescriptorSet cameraDescriptorWriteSet = {};
		cameraDescriptorWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraDescriptorWriteSet.dstSet = cameraSet;
		cameraDescriptorWriteSet.dstBinding = 1;
		cameraDescriptorWriteSet.dstArrayElement = 0;
		cameraDescriptorWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraDescriptorWriteSet.descriptorCount = 1;
		cameraDescriptorWriteSet.pBufferInfo = &cameraViewBufferInfo;
		camerawrites.push_back(cameraDescriptorWriteSet);

		vkUpdateDescriptorSets(renderer->logicalDevice,	camerawrites.size(), camerawrites.data(), 0, nullptr);
	}

	void Motor::CreateDescriptorSets()
	{

		for (int i = 0; i < renderables.size(); i++)
		{
			VkDescriptorSetLayout layouts[] = { descriptorSetlayout };

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = layouts;

			VkResult result = vkAllocateDescriptorSets(renderer->logicalDevice, &allocInfo, renderables.Get(i)->GetDescriptorSet());
			if (result != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to allocate descriptorset"); 
			}
			else 
			{ 
				std::cout << "Allocated descriptor succesfully" << std::endl; 
			}

			std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = *renderables.Get(i)->GetUniformBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(glm::mat4);

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = *renderables.Get(i)->GetDescriptorSet();
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			/*Update descriptors for frag shader Sampler2D*/
			if (renderables.Get(i)->textures.size() == 0)
			{
				VkDescriptorImageInfo imageInfo = { };
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = defaultTexture->imageView;
				imageInfo.sampler = defaultSampler;

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = *renderables.Get(i)->GetDescriptorSet();
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &imageInfo;

				vkUpdateDescriptorSets(renderer->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

				return;
			}
			else
			{
				std::vector<VkDescriptorImageInfo> imageInfos;

				for (Texture* var : renderables.Get(i)->textures)
				{
					VkDescriptorImageInfo tmp; 
					tmp.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					tmp.imageView = var->imageView;
					tmp.sampler = defaultSampler;
					imageInfos.push_back(tmp);
				}

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = *renderables.Get(i)->GetDescriptorSet();
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = imageInfos.data();

				vkUpdateDescriptorSets(renderer->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

				return;
			}


		}
	}

	void Motor::CreateTerrainDescriptorSets()
	{
		for (int i = 0; i < terrains.size(); i++)
		{
			VkDescriptorSetLayout layouts[] = { terrainDescriptorSetLayout };

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = layouts;

			VkResult result = vkAllocateDescriptorSets(renderer->logicalDevice, &allocInfo, terrains[i].GetDescriptorSet());
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to allocate descriptorset");
			}
			else
			{
				std::cout << "Allocated descriptor succesfully" << std::endl;
			}

			std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = *terrains[i].GetUniformBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(glm::mat4);

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = *terrains[i].GetDescriptorSet();
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			/*Update descriptors for frag shader Sampler2D*/
			if (terrains[i].textures.size() == 0)
			{
				VkDescriptorImageInfo imageInfo = {};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = defaultTexture->imageView;
				imageInfo.sampler = defaultSampler;

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = *terrains[i].GetDescriptorSet();
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &imageInfo;

				vkUpdateDescriptorSets(renderer->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

				return;
			}
			else
			{
				std::vector<VkDescriptorImageInfo> imageInfos;

				for (Texture* var : terrains[i].textures)
				{
					VkDescriptorImageInfo tmp;
					tmp.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					tmp.imageView = var->imageView;
					tmp.sampler = defaultSampler;
					imageInfos.push_back(tmp);
				}

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = *terrains[i].GetDescriptorSet();
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 4;
				descriptorWrites[1].pImageInfo = imageInfos.data();

				vkUpdateDescriptorSets(renderer->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

				return;
			}
		}
	}


	VkShaderModule Motor::CreateShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();

		std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
		memcpy(codeAligned.data(), code.data(), code.size());
		createInfo.pCode = codeAligned.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(renderer->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
		}
		else
		{
			std::cout << "Shader module created successfully" << std::endl;
		}
		return shaderModule;
	}

	VkShaderModule Motor::CreateShaderModule(const std::vector<unsigned int>& code)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();

		std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
		memcpy(codeAligned.data(), code.data(), code.size());
		createInfo.pCode = codeAligned.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(renderer->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
		}
		else
		{
			std::cout << "Shader module created successfully" << std::endl;
		}
		return shaderModule;
	}
	
	void Motor::LightVisibilityBuffer()
	{

	}





	void Motor::CreateRenderable(const char* name, const char* model, std::vector<const char*> texture)
	{
		std::vector<Texture*> tmpTexVec;
		for(int i = 0; i < texture.size(); i++)
		{
			tmpTexVec.push_back(textures.Get(texture[i]));
		}

		renderables.Push(name, RenderableObject(renderer, models.Get(model), tmpTexVec));
		std::cout << "test";
	}

	

	void Motor::CheckRSS()
	{
		for (SaveStructure var : RSS.saveData)
		{
			CreateRenderable(var.name.c_str(), var.model.c_str(), var.texture.c_str());
			renderables.Get(var.name.c_str())->Transform = var.transformation;
		}
	}
}