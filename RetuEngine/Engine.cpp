#include "Engine.h"
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

	struct _Dummy_VisibleLightsForTile
	{
		uint32_t count;
		std::array<uint32_t, 63> lightindices;
	};

	Engine::~Engine()
	{
		delete renderer;
		delete swapChain;
		//for (int i = 0; i < renderables.size(); i++)
		//{
		//	delete renderables[i];
		//}
		delete camera;
		delete inputManager;
	}

	void Engine::InitVulkan()
	{
		InitWindow(); //Create GLFW window
		CreateInstance(); //Create vulkan instance
		SetupDebugCallback(); //Get vulkan call when something breaks up
		renderer = new RenderInterface(instance, &windowObj); //renderinterface holds in multible parts of vulkan.
		swapChain = new SwapChain(); 
		swapChain->Create(renderer);
		CreateRenderPass();
		CreateDescriptorSetlayout();
		CreateGraphicsPipeline();
		LoadTextures();
		swapChain->CreateFrameBuffers(&renderPass); 

		std::vector<Vertex> vertices =
		{
			//{ { 0.0f, 0.0f,1.5f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } ,{1.0f,1.0f,0.0f}},
			//{ { 1.0f, 0.0f,1.5f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } ,{1.0f,1.0f,0.0f}},
			//{ { 1.0f, 1.0f,1.5f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } ,{1.0f,1.0f,0.0f}},
			//{ { 0.0f, 1.0f,1.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } ,{1.0f,1.0f,0.0f}}
			{ { 0.0f, 0.0f,0.0f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } ,{0.0f,1.0f,0.0f}},
			{ { 1.0f, 0.0f,0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } ,{0.0f,1.0f,0.0f}},
			{ { 1.0f, 0.0f,1.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f } ,{0.0f,1.0f,0.0f}},
			{ { 0.0f, 0.0f,1.0f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 1.0f } ,{0.0f,1.0f,0.0f}}
		};

		//renderables.push_back(new RenderableObject(renderer, camera,vertices,"Weeb.bmp"));
		//renderables.push_back(new RenderableObject(renderer, camera,"Pekka.bmp"));
		

		for (int i = 0; i < 1; i++)
		{
			vertices[0].pos.x += 1;
			vertices[1].pos.x += 1;
			vertices[2].pos.x += 1;
			vertices[3].pos.x += 1;
			renderables.push_back(Sprite(renderer, camera, vertices, textures.Get("Weeb")));
		}

		//renderables.push_back(Model(renderer, camera, "chalet.obj", textures.Get("chalet")));

		//renderables.push_back(Model(renderer, camera, "Bunny.obj", textures.Get("chalet")));

		//Model* model = new Model(renderer, 600, 800, "chalet.obj");
		//delete model;
		createTextureSampler();
		CreateDescriptorPool();
		CreateLights();
		CreateLightCullingDescriptorSet();
		LightVisibilityBuffer();
		CreateDescriptorSets();
		CreateCommandBuffers();
		CreateSemaphores();
		GameLoop();
	}

	void Engine::CleanUpVulkan()
	{
		CleanUpSwapChain();
		textures.CleanUp();
		vkDestroySampler(renderer->logicalDevice, defaultSampler, nullptr);
		vkDestroyDescriptorPool(renderer->logicalDevice, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(renderer->logicalDevice, descriptorSetlayout, nullptr);
		for (RenderableObject rend : renderables)
		{
			rend.CleanUp(&renderer->logicalDevice);
		}
		vkDestroySemaphore(renderer->logicalDevice,renderFinishedSemaphore,VK_NULL_HANDLE);
		vkDestroySemaphore(renderer->logicalDevice,imageAvailableSemaphore,VK_NULL_HANDLE);
		renderer->CleanCommandPools();
		vkDestroyDevice(renderer->logicalDevice, VK_NULL_HANDLE);
		DestoyDebugReportCallbackEXT(instance, callback, VK_NULL_HANDLE);
		vkDestroySurfaceKHR(instance, renderer->surface, VK_NULL_HANDLE);
		vkDestroyInstance(instance, VK_NULL_HANDLE);
		glfwDestroyWindow(windowObj.window);
		glfwTerminate();
	}

	void Engine::CleanUpSwapChain()
	{

		swapChain->CleanUpFrameBuffers();
		vkFreeCommandBuffers(renderer->logicalDevice,*renderer->GetCommandPool(),static_cast<uint32_t>(commandBuffers.size()),commandBuffers.data());
		vkDestroyPipeline(renderer->logicalDevice, graphicsPipeline, VK_NULL_HANDLE);
		vkDestroyPipelineLayout(renderer->logicalDevice,pipelineLayout,VK_NULL_HANDLE);
		vkDestroyRenderPass(renderer->logicalDevice,renderPass,VK_NULL_HANDLE);

		swapChain->CleanUp();
	}

	void Engine::GameLoop()
	{
		while (!glfwWindowShouldClose(windowObj.window))
		{
			glfwPollEvents();
			for (RenderableObject var : renderables)
			{
				var.UpdateUniform(&renderer->logicalDevice, *swapChain->GetExtent());
			}
			DrawFrame();

			float cameraSpeed = 0.005f; // adjust accordingly
			if (glfwGetKey(windowObj.window, GLFW_KEY_W) == GLFW_PRESS)
				inputManager->camera->cameraPos += cameraSpeed * inputManager->camera->cameraFront;
			if (glfwGetKey(windowObj.window, GLFW_KEY_S) == GLFW_PRESS)
				inputManager->camera->cameraPos -= cameraSpeed * inputManager->camera->cameraFront;
			if (glfwGetKey(windowObj.window, GLFW_KEY_A) == GLFW_PRESS)
			{
				glm::vec3 cross = glm::cross(inputManager->camera->cameraFront, inputManager->camera->cameraUp);
				if (cross == glm::vec3(0, 0, 0))
				{
					inputManager->camera->cameraPos.x -= cameraSpeed;
				}
				else 
				{
					glm::vec3 norm = glm::normalize(cross); 
					inputManager->camera->cameraPos -= norm*cameraSpeed;
				}			
			}
			if (glfwGetKey(windowObj.window, GLFW_KEY_D) == GLFW_PRESS)
			{
				glm::vec3 cross = glm::cross(inputManager->camera->cameraFront, inputManager->camera->cameraUp);
				if (cross == glm::vec3(0, 0, 0))
				{
					inputManager->camera->cameraPos.x += cameraSpeed;
				}
				else 
				{
					glm::vec3 norm = glm::normalize(cross); 
					inputManager->camera->cameraPos += norm*cameraSpeed;
				}
			}
			if (glfwGetKey(windowObj.window, GLFW_KEY_SPACE) == GLFW_PRESS)
				inputManager->camera->cameraPos.y += cameraSpeed;
			if (glfwGetKey(windowObj.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				inputManager->camera->cameraPos.y -= cameraSpeed;
			if (glfwGetKey(windowObj.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwDestroyWindow(windowObj.window);
				glfwTerminate();
			}
		}
		vkDeviceWaitIdle(renderer->logicalDevice);
		CleanUpVulkan();
	}
	
	void Engine::InitWindow()
	{
		windowObj.Create();
		glfwSetWindowUserPointer(windowObj.window, this);
		glfwSetWindowSizeCallback(windowObj.window, Engine::OnWindowResized);
		glfwSetInputMode(windowObj.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		camera = new Camera();
		glfwSetCursorPosCallback(windowObj.window, mouse_callback);
		inputManager = new Input(windowObj.window,windowObj.height, windowObj.width,camera);
	}

	void Engine::LoadTextures()
	{
		Texture tempTexture("chalet.jpg",renderer);	//Textures for the house;
		textures.Push("chalet", tempTexture);

		tempTexture = Texture("Weeb.bmp", renderer); //Texture for weeb sprite;
		textures.Push("Weeb", tempTexture);
	}

	void Engine::AddRenderable(glm::vec3 position, RenderableObject renderable)
	{
		renderables.push_back(renderable);
	}

	void Engine::ReCreateSwapChain()
	{
		vkDeviceWaitIdle(renderer->logicalDevice);
		CleanUpSwapChain();
		swapChain->Create(renderer);
		CreateRenderPass();
		CreateGraphicsPipeline();
		swapChain->CreateFrameBuffers(&renderPass);
		CreateCommandBuffers();
	}

	void Engine::CreateInstance()
	{
		if (enableValidationLayers && !CheckValidationLayerSupport())
		{
			throw std::runtime_error("Validation layer requested but are not available");
		}
		VkApplicationInfo appInfo = {};
		
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "VulkanApp";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;
		
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &appInfo;

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}

		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		if (result)
		{
			throw std::runtime_error("Failed to create vulkan instance");
		}
		else
		{
			std::cout << "Vulkan instance created successfully" << std::endl;
		}
	}

	bool Engine::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount,nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		for (const char* layername : validationLayers)
		{
			bool layerFound = false;
			for (const auto &layerProperties : availableLayers)
			{
				if (strcmp(layername, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}
			if (!layerFound) { return false; }
		}
		return true;
	}
	std::vector<const char*> Engine::GetRequiredExtensions()
	{
		std::vector<const char*> extensions;
		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		for (unsigned int i = 0; i < glfwExtensionCount; i++)
		{
			extensions.push_back(glfwExtensions[i]);
		}
		if (enableValidationLayers) { extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); }
		return extensions;
	}

	VkResult Engine::CreateDebugReportReportCallbackEXT( VkInstance istance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) { return func(instance, pCreateInfo, pAllocator, pCallback); }
		else { return VK_ERROR_EXTENSION_NOT_PRESENT; }
	}

	void Engine::DestoyDebugReportCallbackEXT( VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr)
		{
			func(instance, callback, pAllocator);
		}
	}

	void Engine::SetupDebugCallback()
	{
		if (!enableValidationLayers)
		{
			return;
		}
		VkDebugReportCallbackCreateInfoEXT createInfo = {  };
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (CreateDebugReportReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to setup debug callback");
		}
		else
		{
			std::cout << "Debug callback setup successfull" << std::endl;
		}
	}

	void Engine::CreateRenderPass()
	{
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = *swapChain->GetDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = *swapChain->GetImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassCreateInfo.pAttachments = attachments.data();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(renderer->logicalDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create renderPass");
		}
		else
		{
			std::cout << "Created RenderPass successfully" << std::endl;
		}

	}

	void Engine::CreateDescriptorSetlayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0; 
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //uniform buffer. Uniform lisätty shaderiin
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //vertex shaderille

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; //kerrotaan että bindattava on texshader
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; //fragment shaderille


		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(renderer->logicalDevice, &layoutInfo, nullptr, &descriptorSetlayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create DescriptorLayout");
		}
		else
		{
			std::cout << "Created Descriptor Layout Successfully" << std::endl;
		}

		std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {};

		{
			// create descriptor for storage buffer for light culling results
			//VkDescriptorSetLayoutBinding lb = {};
			//lb.binding = 0;
			//lb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			//lb.descriptorCount = 1;
			//lb.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			//lb.pImmutableSamplers = nullptr;
			//setLayoutBinding.push_back(lb);
		}

		{
			// uniform buffer for point lights
			VkDescriptorSetLayoutBinding lb = {};
			lb.binding = 0;
			lb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // FIXME: change back to uniform
			lb.descriptorCount = 1;  // maybe we can use this for different types of lights
			lb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			lb.pImmutableSamplers = nullptr;
			setLayoutBinding.push_back(lb);
		}

		VkDescriptorSetLayoutCreateInfo layout_info = {};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.pNext = NULL;
		layout_info.flags = 0;
		layout_info.bindingCount = 1;
		layout_info.pBindings = &setLayoutBinding[0];

		//layout_info.bindingCount = static_cast<uint32_t>(setLayoutBinding.size());
		//layout_info.pBindings = setLayoutBinding.data();


		if (vkCreateDescriptorSetLayout(renderer->logicalDevice, &layout_info, nullptr, &lightDescriptorSetlayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create light descriptor layout");
		}

	}

	void Engine::CreateLights()
	{
		//for (int i = 0; i < 10; i++) {
		//	glm::vec3 color;
		//	do { color = { glm::linearRand(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)) }; } while (color.length() < 0.8f);
		//	pointLights.emplace_back(glm::linearRand(glm::vec3(1,1,1), glm::vec3(10,10,10)), 10, 10);
		//}
		
		for (int i = 0; i < 10; i++)
		{
			Pointlight tmp(glm::vec3(i,0.5f,0), 10, glm::vec3(1,1,1));
			pointLights.push_back(tmp);
		}

		auto lightNum = static_cast<int>(pointLights.size());

		pointLightBuffer.bufferSize = sizeof(Pointlight) * 100 + sizeof(glm::vec4);

		lightsStagingBuffer.CreateBuffer(&renderer->logicalDevice,&renderer->physicalDevice,&renderer->surface,pointLightBuffer.bufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		pointLightBuffer.CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, pointLightBuffer.bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	void Engine::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 3> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1 + renderables.size();
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = renderables.size();
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount = 3;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(renderables.size()*2 + 1);

		if (vkCreateDescriptorPool(renderer->logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Descriptor Pool");
		}
		else
		{
			std::cout << "Created Descriptor Pool Successfully" << std::endl;
		}
	}

	void Engine::CreateLightCullingDescriptorSet()
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
	}

	void Engine::CreateDescriptorSets()
	{
		for (int i = 0; i < renderables.size(); i++)
		{
			VkDescriptorSetLayout layouts[] = { descriptorSetlayout };

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = layouts;

			VkResult result = vkAllocateDescriptorSets(renderer->logicalDevice, &allocInfo, renderables[i].GetDescriptorSet());
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
			bufferInfo.buffer = *renderables[i].GetUniformBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = renderables[i].GetUniformBufferSize();

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = *renderables[i].GetDescriptorSet();
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = renderables[i].texture->imageView;
			imageInfo.sampler = defaultSampler;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = *renderables[i].GetDescriptorSet();
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(renderer->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void Engine::CreateGraphicsPipeline()
	{
		auto fragShaderCode = ReadFile("shaders/frag.spv");
		auto vertShaderCode = ReadFile("shaders/vert.spv");

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		vertShaderModule = CreateShaderModule(vertShaderCode);
		fragShaderModule = CreateShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderCreateInfo = {  };
		vertShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderCreateInfo.module = vertShaderModule;
		vertShaderCreateInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderCreateInfo = {  };
		fragShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderCreateInfo.module = fragShaderModule;
		fragShaderCreateInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderCreateInfo,fragShaderCreateInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescription = Vertex::getAttributeDescription();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = false;

		VkViewport viewport = {  };
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChain->GetExtent()->width;
		viewport.height = (float)swapChain->GetExtent()->height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = *swapChain->GetExtent();

		VkPipelineViewportStateCreateInfo viewportCreateInfo = { };
		viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportCreateInfo.viewportCount = 1;
		viewportCreateInfo.pViewports = &viewport;
		viewportCreateInfo.scissorCount = 1;
		viewportCreateInfo.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_LINE_WIDTH };

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
		dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.dynamicStateCount = 2;
		dynamicStateCreateInfo.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		std::array<VkDescriptorSetLayout, 2> setLayouts = { descriptorSetlayout,lightDescriptorSetlayout };
		pipelineLayoutInfo.setLayoutCount = static_cast<int>(setLayouts.size());
		pipelineLayoutInfo.pSetLayouts = setLayouts.data();

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};

		if (vkCreatePipelineLayout(renderer->logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}
		else
		{
			std::cout << "Pipeline Layout Created successfully" << std::endl;
		}

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount = 2;
		pipelineCreateInfo.pStages = shaderStages;
		pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
		pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
		pipelineCreateInfo.pViewportState = &viewportCreateInfo;
		pipelineCreateInfo.pRasterizationState = &rasterizer;
		pipelineCreateInfo.pMultisampleState = &multisampling;
		pipelineCreateInfo.pColorBlendState = &colorBlending;
		pipelineCreateInfo.layout = pipelineLayout;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.pDepthStencilState = &depthStencil;
		pipelineCreateInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(renderer->logicalDevice,VK_NULL_HANDLE,1,&pipelineCreateInfo,nullptr,&graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphicspipeline");
		}
		else
		{
			std::cout << "Created graphics pipeline successfully" << std::endl;
		}
		vkDestroyShaderModule(renderer->logicalDevice,vertShaderModule,VK_NULL_HANDLE);
		vkDestroyShaderModule(renderer->logicalDevice,fragShaderModule,VK_NULL_HANDLE);
	}

	VkShaderModule Engine::CreateShaderModule(const std::vector<char>& code)
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

	void Engine::LightVisibilityBuffer()
	{
		int tileCountPerRow = swapChain->GetExtent()->width - 1 / 16 + 1;
		int tileCountPerCol = swapChain->GetExtent()->height - 1 / 16 + 1;

		VkDeviceSize lightBufferSize = sizeof(_Dummy_VisibleLightsForTile) * tileCountPerCol * tileCountPerRow;

		lightBuffer.CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, lightBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkBuffer testB;
		VkDeviceMemory bufferMem;

		//VkDescriptorBufferInfo lightVisibilityBufferInfo = {};
		//lightVisibilityBufferInfo.buffer = *lightBuffer.GetBuffer();
		//lightVisibilityBufferInfo.offset = 0;
		//lightVisibilityBufferInfo.range = lightBufferSize;

		VkDescriptorBufferInfo pointLightBufferInfo = {};
		pointLightBufferInfo.buffer = *pointLightBuffer.GetBuffer();
		pointLightBufferInfo.offset = 0;
		pointLightBufferInfo.range = pointLightBuffer.bufferSize;


		std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};

		//descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//descriptorWrites[0].dstSet = lightDescriptor;
		//descriptorWrites[0].dstBinding = 0;
		//descriptorWrites[0].dstArrayElement = 0;
		//descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		//descriptorWrites[0].descriptorCount = 1;
		//descriptorWrites[0].pBufferInfo = &lightVisibilityBufferInfo;

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = lightDescriptor;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &pointLightBufferInfo;

		vkUpdateDescriptorSets(renderer->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void Engine::CreateCommandBuffers()
	{
		commandBuffers.resize(swapChain->GetFramebufferSize());
		
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = *renderer->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(renderer->logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command buffers");
		}
		else
		{
			std::cout << "Created command buffers successfully" << std::endl;
		}

		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = *swapChain->GetFramebuffer(i);
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = *swapChain->GetExtent();

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.0f,0.0f,0.0f,1.0f };
			clearValues[1].depthStencil = { 1.0f,0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			for (int j = 0; j < renderables.size(); j++)
			{
				VkBuffer vertexBuffers[] = { *renderables[j].GetVertexBuffer() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
				VkBuffer indexBuffer = *renderables[j].GetIndexBuffer();
				VkDeviceSize indexOffsets[] = { 0 };
				vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, renderables[j].GetDescriptorSet(), 0, nullptr);

				//TODO: broken function. descriptor is empty
				vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(renderables[j].GetIndicesSize()), 1, 0, 0, 0);
			}
			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to end command buffer");
			}
		}
	}

	void Engine::CreateSemaphores()
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(renderer->logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS || 
			vkCreateSemaphore(renderer->logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create semaphore");
		}
		else
		{
			std::cout << "Created semaphores successfully" << std::endl;
		}
	}

	void Engine::DrawFrame()
	{
		uint32_t imageIndex;

		vkAcquireNextImageKHR(renderer->logicalDevice, *swapChain->GetSwapchain(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		
		VkSemaphore waitSemaphore[] = { imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphore;
		submitInfo.pWaitDstStageMask = waitStages;

		VkSemaphore signalSemaphore[] = { renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;	
		submitInfo.pSignalSemaphores = signalSemaphore;	
		submitInfo.commandBufferCount = 1;	
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];	

		if (vkQueueSubmit(renderer->displayQueue,1,&submitInfo,VK_NULL_HANDLE) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit");
		}
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphore;

		
		VkSwapchainKHR swapChains[] = {*swapChain->GetSwapchain()};
		presentInfo.swapchainCount = 1;	
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(renderer->displayQueue, &presentInfo);
	}

	void Engine::createTextureSampler() {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(renderer->logicalDevice, &samplerInfo, nullptr, &defaultSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
}