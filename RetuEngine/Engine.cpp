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

#define MAX_LIGHT_COUNT 200

	struct _Dummy_VisibleLightsForTile
	{
		uint32_t count;
		std::array<uint32_t, 63> lightindices;
	};

	Engine::~Engine()
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
		LoadModels();
		swapChain->CreateFrameBuffers(&renderPass); 
		createTextureSampler();
		//RSS = RenderableSaveSystem("default.sav");
		//CreateRenderable("talo", "chalet", "chalet");
		//CreateRenderable("hill", "hill", "grass");
		//CreateRenderable("valley", "valley", "grass");
		CreateRenderable("cube", "cube", {"weeb","cube-uv"});

		//RSS.AppendToSaveFile("talo", renderables.Get("talo"));
		//RSS.AppendToSaveFile("nibbers", renderables.Get("hill"));
		//RSS.LoadAll();
		//CheckRSS();
		//CreateRenderable("cube", "cube", "weeb");

		CreateDescriptorPool();
		CreateDescriptorSets(); //for objects

		CreateLights();
		CreateLightDescriptorSets();  

		CreateCameraDescriptorSets();

		LightVisibilityBuffer();
		CreateCommandBuffers();
		CreateSemaphores();
		GameLoop();
	}

	void Engine::CleanUpVulkan()
	{
		CleanUpSwapChain();
		defaultTexture->CleanUp();

		textures.CleanUp();
		models.CleanUp();

		vkDestroySampler(renderer->logicalDevice, defaultSampler, nullptr);
		vkDestroyDescriptorPool(renderer->logicalDevice, descriptorPool, nullptr);

		vkDestroyDescriptorSetLayout(renderer->logicalDevice, descriptorSetlayout, nullptr);
		vkDestroyDescriptorSetLayout(renderer->logicalDevice, lightDescriptorSetlayout, nullptr);
		vkDestroyDescriptorSetLayout(renderer->logicalDevice, cameraSetLayout, nullptr);

		if (renderables.size() != 0)
		{
			cameraBuffer.CleanUpBuffer();
			cameraViewBuffer.CleanUpBuffer();
			pointLightBuffer.CleanUpBuffer();
		}
		renderables.CleanUp();

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
		float cameraSpeed = 0.005f; // adjust accordingly
		while (!glfwWindowShouldClose(windowObj.window))
		{
			UpdateUniformBuffers();
			glfwPollEvents();
			DrawFrame();

			if (glfwGetKey(windowObj.window, GLFW_KEY_W) == GLFW_PRESS)
				inputManager->camera->cameraPos += cameraSpeed * inputManager->camera->cameraFront;
			if (glfwGetKey(windowObj.window, GLFW_KEY_S) == GLFW_PRESS)
				inputManager->camera->cameraPos -= cameraSpeed * inputManager->camera->cameraFront;
			if (glfwGetKey(windowObj.window, GLFW_KEY_E) == GLFW_PRESS)
				cameraSpeed += 0.001f;
			if (glfwGetKey(windowObj.window, GLFW_KEY_Q) == GLFW_PRESS)
				cameraSpeed -= 0.001f;

			if (glfwGetKey(windowObj.window, GLFW_KEY_O) == GLFW_PRESS)
			{
				system("%cd%/../deps/shaders/compile.bat");
				ReCreateSwapChain();
			}
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
			if (glfwGetKey(windowObj.window, GLFW_KEY_T) == GLFW_PRESS)
			{
				RSS.SaveAll(renderables);
				std::cout << "Scene saved" << std::endl;
				//TODO: overwrite all savedata
			}
			if (glfwGetKey(windowObj.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				inputManager->camera->cameraPos.y -= cameraSpeed;
			if (glfwGetKey(windowObj.window, GLFW_KEY_P) == GLFW_PRESS)
			{
				glfwDestroyWindow(windowObj.window);
				glfwTerminate();
			}
			if (glfwGetKey(windowObj.window, GLFW_KEY_F) == GLFW_PRESS)
			{
				//renderables.Get("talo")->Transform *= glm::translate(glm::mat4(), glm::vec3(0, 0.1f, 0));
			}
			if (glfwGetKey(windowObj.window, GLFW_KEY_K) == GLFW_PRESS)
			{
				pointLights[0].intensity += 0.01f;
				std::cout << pointLights[0].intensity[0] << std::endl;
			}
			if (glfwGetKey(windowObj.window, GLFW_KEY_J) == GLFW_PRESS)
			{
				pointLights[0].intensity -= 0.01f; 
				std::cout << pointLights[0].intensity[0] << std::endl;
			}
			if (glfwGetKey(windowObj.window, GLFW_KEY_N) == GLFW_PRESS)
			{
				pointLights[0].radius -= 0.01f; 
				std::cout << pointLights[0].radius[0] << std::endl;
			}
			if (glfwGetKey(windowObj.window, GLFW_KEY_M) == GLFW_PRESS)
			{
				pointLights[0].radius += 0.01f; 
				std::cout << pointLights[0].radius[0] << std::endl;
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
		defaultTexture = new Texture("default.png", renderer);
		//textures.Push("hill", Texture("MapText.jpg", renderer));
		textures.Push("chalet", Texture("chalet.jpg",renderer));
		textures.Push("weeb", Texture("Weeb.bmp",renderer));
		textures.Push("grass", Texture("grasstex.jpg",renderer));
		textures.Push("cube-uv", Texture("CubeUV.png",renderer,true));
	}

	void Engine::LoadModels()
	{
		models.Push("cube"	, Model(renderer, "Cube.obj"));
		models.Push("hill"	, Model(renderer, "Hill.obj"));
		models.Push("chalet", Model(renderer, "chalet.obj"));
		models.Push("bunny"	, Model(renderer, "Bunny.obj"));
		models.Push("valley", Model(renderer, "Valley.obj"));
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
		samplerLayoutBinding.descriptorCount = 2;
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

		// uniform buffer for point lights
		VkDescriptorSetLayoutBinding lb = {};
		lb.binding = 0;
		lb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // FIXME: change back to uniform
		lb.descriptorCount = 1;  // maybe we can use this for different types of lights
		lb.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT| VK_SHADER_STAGE_FRAGMENT_BIT;
		lb.pImmutableSamplers = nullptr;
		setLayoutBinding.push_back(lb);

		VkDescriptorSetLayoutCreateInfo lightInfo = {};
		lightInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		lightInfo.pNext = NULL;
		lightInfo.flags = 0;
		lightInfo.bindingCount = static_cast<uint32_t>(setLayoutBinding.size());
		lightInfo.pBindings = setLayoutBinding.data();


		if (vkCreateDescriptorSetLayout(renderer->logicalDevice, &lightInfo, nullptr, &lightDescriptorSetlayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create light descriptor layout");
		}


		VkDescriptorSetLayoutBinding cameraLayoutBinding[2] = {};
		cameraLayoutBinding[0].binding = 0;
		cameraLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; //kerrotaan että bindattava on texshader
		cameraLayoutBinding[0].descriptorCount = 1;
		cameraLayoutBinding[0].pImmutableSamplers = nullptr;
		cameraLayoutBinding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; //fragment shaderille

		cameraLayoutBinding[1].binding = 1;
		cameraLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //kerrotaan että bindattava on texshader
		cameraLayoutBinding[1].descriptorCount = 1;
		cameraLayoutBinding[1].pImmutableSamplers = nullptr;
		cameraLayoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //fragment shaderille

		VkDescriptorSetLayoutCreateInfo cameraLayoutInfo = {};
		cameraLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		cameraLayoutInfo.pNext = NULL;
		cameraLayoutInfo.flags = 0;
		cameraLayoutInfo.bindingCount = 2;
		cameraLayoutInfo.pBindings = cameraLayoutBinding;

		if (vkCreateDescriptorSetLayout(renderer->logicalDevice, &cameraLayoutInfo, nullptr, &cameraSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create light descriptor layout");
		}

	}

	void Engine::CreateLights()
	{
		for (int i = 0; i < 1; i++) {
			// do { color = { glm::linearRand(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)) }; } while (color.length() < 0.8f);
			
			pointLights.emplace_back(glm::linearRand(glm::vec3(1,1,0), glm::vec3(1,1,0)), 7.5, -1.5,glm::vec4(1,1,1,1));
		}

		int lightnum = pointLights.size();

		VkDeviceSize size = sizeof(Pointlight) * lightnum + sizeof(glm::vec4);

		pointLightBuffer = Buffer(renderer);
		pointLightBuffer.StartMapping(size,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pointLightBuffer.Map(&lightnum, sizeof(glm::vec4));
		pointLightBuffer.Map(pointLights.data(), sizeof(Pointlight)*pointLights.size());
		pointLightBuffer.StopMapping();

	}

	void Engine::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 3> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 3 + renderables.size();
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 2 + renderables.size();
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount = 100 + 200;

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

	void Engine::CreateLightDescriptorSets()
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

		//int tileCountPerRow = swapChain->GetExtent()->width - 1 / 16 + 1;
		//int tileCountPerCol = swapChain->GetExtent()->height - 1 / 16 + 1;

		//VkDeviceSize lightBufferSize = sizeof(_Dummy_VisibleLightsForTile) * tileCountPerCol * tileCountPerRow;

		//lightBuffer.CreateBuffer(&renderer->logicalDevice, &renderer->physicalDevice, &renderer->surface, lightBufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		//VkBuffer testB;
		//VkDeviceMemory bufferMem;

		//VkDescriptorBufferInfo lightVisibilityBufferInfo = {};
		//lightVisibilityBufferInfo.buffer = *lightBuffer.GetBuffer();
		//lightVisibilityBufferInfo.offset = 0;
		//lightVisibilityBufferInfo.range = lightBufferSize;

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

	void Engine::CreateCameraDescriptorSets()
	{

		if (renderables.size() == 0) return;

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
				descriptorWrites[1].descriptorCount = 2;
				descriptorWrites[1].pImageInfo = imageInfos.data();

				vkUpdateDescriptorSets(renderer->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

				return;
			}


		}
	}

	void Engine::CreateGraphicsPipeline()
	{
		auto fragShaderCode = ReadFile("shaders/frag.spv");
		auto vertShaderCode = ReadFile("shaders/vert.spv");
		//auto fragShaderCode = CompileGLSL("shaders/shader.frag");
		//auto vertShaderCode = CompileGLSL("shaders/shader.vert");
		
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
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;

		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_LINE_WIDTH };

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
		dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.dynamicStateCount = 2;
		dynamicStateCreateInfo.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		std::array<VkDescriptorSetLayout, 3> setLayouts = { descriptorSetlayout,lightDescriptorSetlayout,cameraSetLayout };
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

	VkShaderModule Engine::CreateShaderModule(const std::vector<unsigned int>& code)
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
				VkBuffer vertexBuffers[] = { *renderables[j]->GetVertexBuffer() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
				VkBuffer indexBuffer = *renderables[j]->GetIndexBuffer();
				VkDeviceSize indexOffsets[] = { 0 };
				vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
				std::array<VkDescriptorSet, 3> descriptor_sets = { *renderables[j]->GetDescriptorSet(), lightDescriptor, cameraSet };
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descriptor_sets.size(), descriptor_sets.data(), 0, nullptr);

				vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(renderables[j]->GetIndicesSize()), 1, 0, 0, 0);
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

	void Engine::createTextureSampler() 
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = 10;
		samplerInfo.mipLodBias = 0;

		if (vkCreateSampler(renderer->logicalDevice, &samplerInfo, nullptr, &defaultSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	void Engine::CreateRenderable(const char* name, const char* model, const char* texture)
	{
		renderables.Push(name, RenderableObject(renderer, models.Get(model), textures.Get(texture)));
		std::cout << "test";
	}

	void Engine::CreateRenderable(const char* name, const char* model, std::vector<const char*> texture)
	{
		std::vector<Texture*> tmpTexVec;
		for(int i = 0; i < texture.size(); i++)
		{
			tmpTexVec.push_back(textures.Get(texture[i]));
		}

		renderables.Push(name, RenderableObject(renderer, models.Get(model), tmpTexVec));
		std::cout << "test";
	}

	void Engine::UpdateUniformBuffers()
	{
		if (renderables.size() == 0) return;
		//std::cout << camera->cameraPos.x << " " << camera->cameraPos.y << " " << camera->cameraPos.z << std::endl;
		cameraBuffer.StartUpdate();
		cameraBuffer.UpdateMap(&camera->cameraPos, sizeof(glm::vec3));
		cameraBuffer.StopUpdate(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

		camera->view = glm::lookAt(camera->cameraPos,  camera->cameraPos + camera->cameraFront, camera->cameraUp);
		camera->proj = glm::perspective(glm::radians(90.0f), swapChain->GetExtent()->width / (float)swapChain->GetExtent()->height, 0.1f, 10000.0f);
		camera->proj[1][1] *= -1; //flipping y coordinate?

		//std::cout << camera->cameraPos.x << " " << camera->cameraPos.y << " " << camera->cameraPos.z << std::endl;

		//for cameraViewMatrix at vert
		cameraViewBuffer.StartUpdate();
		cameraViewBuffer.UpdateMap(&camera->view,sizeof(glm::mat4));
		cameraViewBuffer.UpdateMap(&camera->proj,sizeof(glm::mat4));
		cameraViewBuffer.StopUpdate(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		//Light updates
		pointLightBuffer.StartUpdate();
		int pointLightSize = pointLights.size();
		pointLightBuffer.UpdateMap(&pointLightSize,sizeof(glm::vec4));
		pointLightBuffer.UpdateMap(pointLights.data(), sizeof(Pointlight)*pointLightSize);
		pointLightBuffer.StopUpdate(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

		pointLights[0].pos = glm::vec4(camera->cameraPos,1);

		//renderables.Get("cube")->Transform = glm::translate(glm::mat4(), glm::vec3(camera->cameraPos.x,camera->cameraPos.y,camera->cameraPos.z));

		for (int i = 0; i < renderables.size(); i++)
		{
			renderables[i]->UpdateUniform();
		}
	}

	void Engine::CheckRSS()
	{
		for (SaveStructure var : RSS.saveData)
		{
			CreateRenderable(var.name.c_str(), var.model.c_str(), var.texture.c_str());
			renderables.Get(var.name.c_str())->Transform = var.transformation;
		}
	}
}