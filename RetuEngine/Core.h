#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <vector>
#include <cstring> //Not sure of usage
#include <iostream>

//Core includes
#include "Input.h" //Window and camera also
#include "RenderInterface.h"
#include "SwapChain.h"
#include "Utility.h"

#include "Texture.h"
#include "CubeMap.h"
#include "Model.h"
#include "Terrain.h"
#include "RenderableObject.h"

/* Change Log

	-Removed default constructor and destructor;
	-Removed InitWindow() and put glfw instance creation inside window class
	-Moved CreateVulkanInstance() to RenderInterface

*/

namespace Engine
{
	
	class Core
	{
	/*Public Functions*/
	public:
		
		void InitVulkan();
		void ReCreateSwapChain();
		void CreateRenderable(const char* name, const char* model, const char* texture);
		void CreateTerrain(const char* name, const char* model, std::vector <const char*> texture);

	/*Private Functions*/
	private:

		void CreateRenderPass();
		void CreateDescriptorSetlayout();
		void CreateTerrainDescriptorSetlayout();
		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		VkShaderModule CreateShaderModule(const std::vector<unsigned int>& code);

		void CreateGraphicsPipeline();
		void CreateTerrainPipeline();
		void CreateCubemapPipeline();

		void CreateTextureSampler();

		void LoadTextures();
		void LoadModels();

		void CreateDescriptorPool();

		void CreateLights();

		void CreateDescriptorSets();
		void CreateLightDescriptorSets();
		void CreateCameraDescriptorSets();
		void CreateTerrainDescriptorSets();

		void CreateCommandBuffers();	
		void CreateSemaphores();

		void GameLoop();
		void UpdateUniformBuffers();
		void DrawFrame();

		void CleanUpSwapChain();

		void CleanUpVulkan();


	/*Public Variables*/
	public:
		Window window; //Stores GLFWinstance and size
		Input* inputManager;

	/*Private Variables*/
	private:

		RenderInterface* vulkanInterface;
		SwapChain* swapChain;

		VkRenderPass renderPass;

		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

		VkPipelineLayout terrainPipelineLayout;
		VkPipeline terrainPipeline;

		VkPipelineLayout cubemapPipelineLayout;
		VkPipeline cubemapPipeline;

		VkDescriptorSetLayout descriptorSetlayout;
		VkDescriptorSetLayout lightDescriptorSetlayout;
		VkDescriptorSetLayout cameraSetLayout;
		VkDescriptorSetLayout terrainDescriptorSetLayout;

		VkDescriptorPool descriptorPool;

		VkSampler defaultSampler;

		Texture* defaultTexture;
		TextureVector textures;

		ModelVector models;

		std::vector<Terrain> terrains;
		RenderableVector renderables;
		RenderableObject skyboxRO;
		CubeMap* cubemappi;

		std::vector<Pointlight> pointLights;
		Buffer pointLightBuffer;

		/*Descriptors*/
		VkDescriptorSet cubemapDescriptorSet;
		VkDescriptorSet lightDescriptor;
		VkDescriptorSet cameraSet;

		Buffer cameraBuffer;
		Buffer cameraViewBuffer;
		Buffer skyboxCameraBuffer;

		std::vector<VkCommandBuffer> commandBuffers;
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;

	};

	static void OnWindowResized(GLFWwindow* window,int width, int height)
	{
		if (width == 0 || height == 0)
		{
			return;
		}
		Core* engine(reinterpret_cast<Core*>(glfwGetWindowUserPointer(window))); //getting pointer to core
		engine->window.Resize(width, height); //saving new size to window class
		engine->ReCreateSwapChain();
	}

	static void CursorCallback(GLFWwindow*window, double xpos, double ypos)
	{
		Core* engine = reinterpret_cast<Core*>(glfwGetWindowUserPointer(window));
		engine->inputManager->UpdateCamera(xpos, ypos);
	}

}

