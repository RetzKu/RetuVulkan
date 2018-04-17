#include "Engine.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include <map>
#include <algorithm>
#include <set>

namespace RetuEngine
{
	Engine::Engine()
	{
	}

	Engine::~Engine()
	{
		delete swapChain;
		delete graphicsCommandPool;
		delete transferCommandPool;
		delete uniformBuffer;
		delete indexBuffer;
		delete vertexBuffer;
		delete mainRectangle;
	}

	void Engine::InitVulkan()
	{
		InitWindow();
		CreateInstance();
		SetupDebugCallback();
		CreateSurface();
		GetPhysicalDevices();
		CreateLogicalDevice();
		swapChain = new SwapChain();
		swapChain->Create(&logicalDevice,&physicalDevice,&surface,&windowObj,QuerySwapChainSupport(physicalDevice));
		CreateRenderPass();
		CreateDescriptorSetlayout();
		CreateGraphicsPipeline();
		swapChain->CreateFrameBuffers(&renderPass);
		QueueFamilyIndices indices = FindQueueFamilies(&physicalDevice, &surface);
		graphicsCommandPool = new CommandPool(&logicalDevice,indices.displayFamily);
		transferCommandPool = new CommandPool(&logicalDevice,indices.transferFamily,VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		vertexBuffer = new VertexBuffer(&logicalDevice, &physicalDevice, &surface, transferCommandPool->GetCommandPool(), &transferQueue);
		indexBuffer = new IndexBuffer(&logicalDevice, &physicalDevice, &surface, transferCommandPool->GetCommandPool(), &transferQueue);
		uniformBuffer = new UniformBuffer(&logicalDevice, &physicalDevice, &surface, transferCommandPool->GetCommandPool(), &transferQueue);
		mainRectangle = new Gameobject(&logicalDevice, &physicalDevice, &surface, transferCommandPool->GetCommandPool(), &transferQueue); //EYY gameobjects man
		CreateDescriptorPool();
		CreateDescriptorSet();
		CreateCommandBuffers();
		CreateSemaphores();
		GameLoop();
	}

	void Engine::CleanUpVulkan()
	{
		CleanUpSwapChain();
		vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetlayout, nullptr);
		uniformBuffer->CleanUp(&logicalDevice);
		indexBuffer->CleanUp(&logicalDevice);
		vertexBuffer->CleanUp(&logicalDevice);
		vkDestroySemaphore(logicalDevice,renderFinishedSemaphore,VK_NULL_HANDLE);
		vkDestroySemaphore(logicalDevice,imageAvailableSemaphore,VK_NULL_HANDLE);
		graphicsCommandPool->CleanUp(&logicalDevice);
		transferCommandPool->CleanUp(&logicalDevice);
		vkDestroyDevice(logicalDevice, VK_NULL_HANDLE);
		DestoyDebugReportCallbackEXT(instance, callback, VK_NULL_HANDLE);
		vkDestroySurfaceKHR(instance, surface, VK_NULL_HANDLE);
		vkDestroyInstance(instance, VK_NULL_HANDLE);
		glfwDestroyWindow(windowObj.window);
		glfwTerminate();
	}

	void Engine::CleanUpSwapChain()
	{
		swapChain->CleanUpFrameBuffers();
		vkFreeCommandBuffers(logicalDevice,*graphicsCommandPool->GetCommandPool(),static_cast<uint32_t>(commandBuffers.size()),commandBuffers.data());
		vkDestroyPipeline(logicalDevice, graphicsPipeline, VK_NULL_HANDLE);
		vkDestroyPipelineLayout(logicalDevice,pipelineLayout,VK_NULL_HANDLE);
		vkDestroyRenderPass(logicalDevice,renderPass,VK_NULL_HANDLE);

		swapChain->CleanUp();
	}

	void Engine::GameLoop()
	{
		while (!glfwWindowShouldClose(windowObj.window))
		{
			glfwPollEvents();
			uniformBuffer->Update(&logicalDevice,*swapChain->GetExtent());
			DrawFrame();
		}
		vkDeviceWaitIdle(logicalDevice);
		CleanUpVulkan();
	}
	
	void Engine::InitWindow()
	{
		windowObj.Create();
		glfwSetWindowUserPointer(windowObj.window, this);
		glfwSetWindowSizeCallback(windowObj.window, Engine::OnWindowResized);
	}

	void Engine::ReCreateSwapChain()
	{
		vkDeviceWaitIdle(logicalDevice);
		CleanUpSwapChain();
		swapChain->Create(&logicalDevice, &physicalDevice, &surface, &windowObj, QuerySwapChainSupport(physicalDevice));
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
	void Engine::CreateSurface()
	{
		if (glfwCreateWindowSurface(instance,windowObj.window , nullptr,&surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface");
		}
		else
		{
			std::cout << "Window surface created successfully" << std::endl;
		}
	}

	SwapChainSupportDetails Engine::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, nullptr);

		if (presentCount != 0)
		{
			details.presentmodes.resize(presentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, details.presentmodes.data());
		}

		return details;
	}

	void Engine::GetPhysicalDevices()
	{
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		if (physicalDeviceCount == 0)
		{
			throw std::runtime_error("No devices with Vulkan support");
		}
		std::vector<VkPhysicalDevice> foundPhysicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, foundPhysicalDevices.data());

		std::multimap<int, VkPhysicalDevice> rankedDevices;

		for (const auto& currentDevice : foundPhysicalDevices)
		{
			int score = RateDeviceSuitability(currentDevice);
			rankedDevices.insert(std::make_pair(score, currentDevice));
		}

		if (rankedDevices.rbegin()->first > 0)
		{
			physicalDevice = rankedDevices.rbegin()->second;
			std::cout << "PhysicalDevice Found" << std::endl;
		}
		else
		{
			throw std::runtime_error("No Physical devices that support Vulkan");
		}
	}

	int Engine::RateDeviceSuitability(VkPhysicalDevice deviceToRate)
	{
		int score = 0;

		QueueFamilyIndices indices = FindQueueFamilies(&deviceToRate, &surface);
		bool extensionsSupported = CheckDeviceExtensionSupport(deviceToRate);
		if (indices.isComplete() == false || !extensionsSupported)
		{
			return 0;
		}

		bool swapChainAdequate = false;
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(deviceToRate);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentmodes.empty();
		if (!swapChainAdequate)
		{
			return 0;
		}

		VkPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(deviceToRate, &deviceProperties);
		vkGetPhysicalDeviceFeatures(deviceToRate, &deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		score += deviceProperties.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader)
		{
			return 0;
		}
		return score;
	}

	void Engine::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(&physicalDevice, &surface);
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.displayFamily, indices.transferFamily };
		const float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = { };
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}


		VkPhysicalDeviceFeatures deviceFeatures = { };

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

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
		createInfo.enabledExtensionCount = deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device");
		}
		else
		{
			std::cout << "Logical device created successfully" << std::endl;
		}
		vkGetDeviceQueue(logicalDevice,indices.displayFamily,0,&displayQueue);
		vkGetDeviceQueue(logicalDevice,indices.transferFamily,0,&transferQueue);
	}




	bool Engine::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensioncount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensioncount, nullptr);

		std::vector<VkExtensionProperties> availableExtension(extensioncount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensioncount, availableExtension.data());

		for (const char* currentextension : deviceExtensions)
		{
			bool extensionFound = false;
			for (const auto& extension : availableExtension)
			{
				if (strcmp(currentextension, extension.extensionName) == 0)
				{
					extensionFound = true;
				}
			}
			if (!extensionFound)
			{
				return false;
			}
		}
		return true;
	}



	void Engine::CreateRenderPass()
	{
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

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &colorAttachment;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(logicalDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
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
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetlayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create DescriptorLayout");
		}
		else
		{
			std::cout << "Created Descriptor Layout Successfully" << std::endl;
		}
	}

	void Engine::CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = 1;

		if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Descriptor Pool");
		}
		else
		{
			std::cout << "Created Descriptor Pool Successfully" << std::endl;
		}
	}

	void Engine::CreateDescriptorSet()
	{
		VkDescriptorSetLayout layouts[] = { descriptorSetlayout };

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Descriptor Pool");
		}
		else
		{
			std::cout << "Created Descriptor Pool Successfully" << std::endl;
		}
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = *uniformBuffer->GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = uniformBuffer->GetUniformBufferSize();

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(logicalDevice, 1, &descriptorWrite, 0, nullptr);
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
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetlayout;

		if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
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
		pipelineCreateInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(logicalDevice,VK_NULL_HANDLE,1,&pipelineCreateInfo,nullptr,&graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphicspipeline");
		}
		else
		{
			std::cout << "Created graphics pipeline successfully" << std::endl;
		}
		vkDestroyShaderModule(logicalDevice,vertShaderModule,VK_NULL_HANDLE);
		vkDestroyShaderModule(logicalDevice,fragShaderModule,VK_NULL_HANDLE);
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
		if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
		}
		else
		{
			std::cout << "Shader module created successfully" << std::endl;
		}
		return shaderModule;
	}

	void Engine::CreateCommandBuffers()
	{
		commandBuffers.resize(swapChain->GetFramebufferSize());
		
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = *graphicsCommandPool->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
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
			VkClearValue clearColor = { 0.0f,0.0f,0.0f,1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			
			VkBuffer vertexBuffers[] = { *vertexBuffer->GetBuffer()};
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

			VkBuffer indexBuffer = *this->indexBuffer->GetBuffer();
			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(this->indexBuffer->GetIndicesSize()), 1, 0, 0, 0);

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

		if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS || 
			vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
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

		vkAcquireNextImageKHR(logicalDevice, *swapChain->GetSwapchain(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

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

		if (vkQueueSubmit(displayQueue,1,&submitInfo,VK_NULL_HANDLE) != VK_SUCCESS)
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

		vkQueuePresentKHR(displayQueue, &presentInfo);
	}
}