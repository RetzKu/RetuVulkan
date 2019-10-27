#include "Core.h"
#include <array>


namespace Engine
{
	/*PUBLIC*/
	void Core::InitVulkan()
	{
		//InitWindow(); //Create GLFW window
		window.Create(this); //Inits GLFW stuff and creates GLFW window for us;

		vulkanInterface = new RenderInterface(&window); //renderinterface holds in multible parts of vulkan.

		swapChain = new SwapChain();
		swapChain->Create(vulkanInterface);

		inputManager = new Input(&window);

		glfwSetWindowSizeCallback(window.Get(), OnWindowResized);
		glfwSetCursorPosCallback(window.Get(), CursorCallback);

		CreateRenderPass();

		CreateDescriptorSetlayout();
		CreateTerrainDescriptorSetlayout();

		CreateGraphicsPipeline();
		CreateTerrainPipeline();
		CreateCubemapPipeline();

		LoadTextures();
		LoadModels();

		swapChain->CreateFrameBuffers(&renderPass);

		CreateTextureSampler();

		//RSS = RenderableSaveSystem("default.sav");

		//Implement these back later and maybe in better position than currently;
		/*
		CreateRenderable("hill", "hill", "grass");
		*/
		//CreateTerrain("terrain", "cube", { "weeb","cube-uv" });

		cubemappi = new CubeMap("ely_hills/hills",vulkanInterface);

		CreateTerrain("valley", "valley", { "grass","valley-blend","dirt","rock","sand" });
		CreateRenderable("talo", "chalet", "chalet");
		//skyboxRO = RenderableObject(vulkanInterface, models.Get("skybox"), textures.Get("skybox"));
		//CreateRenderable("valley", "valley", "grass");
		//CreateRenderable("cube", "cube", {"weeb","cube-uv"});

		//RSS.AppendToSaveFile("talo", renderables.Get("talo"));
		//RSS.AppendToSaveFile("nibbers", renderables.Get("hill"));
		//RSS.LoadAll();
		//CheckRSS();
		//CreateRenderable("cube", "cube", "weeb");

		CreateDescriptorPool();
		CreateDescriptorSets(); //for objects //set 0

		CreateLights();
		CreateLightDescriptorSets();   //set 1

		CreateCameraDescriptorSets(); //set 2

		CreateTerrainDescriptorSets(); //set 3

		CreateCommandBuffers();
		CreateSemaphores();
		GameLoop();
	}

	void Core::CreateRenderable(const char* name, const char* model, const char* texture)
	{
		renderables.Push(name, RenderableObject(vulkanInterface, models.Get(model), textures.Get(texture)));
		std::cout << "test";
	}

	void Core::CreateTerrain(const char* name, const char* model, std::vector <const char*> texture)
	{
		std::vector<Texture*> tmpTexVec;
		for (int i = 0; i < texture.size(); i++)
		{
			tmpTexVec.push_back(textures.Get(texture[i]));
		}

		Terrain tmp(vulkanInterface, models.Get(model), tmpTexVec);
		terrains.push_back(tmp);
	}

	void Core::GameLoop()
	{
		float cameraSpeed = 0.005f; // adjust accordingly

		while (!glfwWindowShouldClose(window.Get()))
		{
			glfwPollEvents();
			UpdateUniformBuffers();
			DrawFrame();

			if (glfwGetKey(window.Get(), GLFW_KEY_W) == GLFW_PRESS)
				inputManager->camera->cameraPos += cameraSpeed * inputManager->camera->cameraFront;
			if (glfwGetKey(window.Get(), GLFW_KEY_S) == GLFW_PRESS)
				inputManager->camera->cameraPos -= cameraSpeed * inputManager->camera->cameraFront;
			if (glfwGetKey(window.Get(), GLFW_KEY_E) == GLFW_PRESS)
				cameraSpeed += 0.001f;
			if (glfwGetKey(window.Get(), GLFW_KEY_Q) == GLFW_PRESS)
				cameraSpeed -= 0.001f;

			if (glfwGetKey(window.Get(), GLFW_KEY_O) == GLFW_PRESS)
			{
				system("%cd%/../deps/shaders/compile.bat");
				ReCreateSwapChain();
			}
			if (glfwGetKey(window.Get(), GLFW_KEY_A) == GLFW_PRESS)
			{
				glm::vec3 cross = glm::cross(inputManager->camera->cameraFront, inputManager->camera->cameraUp);
				if (cross == glm::vec3(0, 0, 0))
				{
					inputManager->camera->cameraPos.x -= cameraSpeed;
				}
				else
				{
					glm::vec3 norm = glm::normalize(cross);
					inputManager->camera->cameraPos -= norm * cameraSpeed;
				}
			}
			if (glfwGetKey(window.Get(), GLFW_KEY_D) == GLFW_PRESS)
			{
				glm::vec3 cross = glm::cross(inputManager->camera->cameraFront, inputManager->camera->cameraUp);
				if (cross == glm::vec3(0, 0, 0))
				{
					inputManager->camera->cameraPos.x += cameraSpeed;
				}
				else
				{
					glm::vec3 norm = glm::normalize(cross);
					inputManager->camera->cameraPos += norm * cameraSpeed;
				}
			}

			//if (glfwGetKey(windowObj.window, GLFW_KEY_T) == GLFW_PRESS)
			//{
			//	RSS.SaveAll(renderables);
			//	std::cout << "Scene saved" << std::endl;
			//	//TODO: overwrite all savedata
			//}

			if (glfwGetKey(window.Get(), GLFW_KEY_SPACE) == GLFW_PRESS)
				inputManager->camera->cameraPos.y += cameraSpeed;
			if (glfwGetKey(window.Get(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				inputManager->camera->cameraPos.y -= cameraSpeed;
			if (glfwGetKey(window.Get(), GLFW_KEY_P) == GLFW_PRESS)
			{
				glfwDestroyWindow(window.Get());
				glfwTerminate();
				vkDeviceWaitIdle(vulkanInterface->logicalDevice);
				CleanUpVulkan();
				return;
			}
			if (glfwGetKey(window.Get(), GLFW_KEY_F) == GLFW_PRESS)
			{
				//renderables.Get("talo")->Transform *= glm::translate(glm::mat4(), glm::vec3(0, 0.1f, 0));
			}
			if (glfwGetKey(window.Get(), GLFW_KEY_K) == GLFW_PRESS)
			{
				pointLights[0].intensity += 0.01f;
				std::cout << pointLights[0].intensity[0] << std::endl;
			}
			if (glfwGetKey(window.Get(), GLFW_KEY_J) == GLFW_PRESS)
			{
				pointLights[0].intensity -= 0.01f;
				std::cout << pointLights[0].intensity[0] << std::endl;
			}
			if (glfwGetKey(window.Get(), GLFW_KEY_N) == GLFW_PRESS)
			{
				pointLights[0].radius -= 0.01f;
				std::cout << pointLights[0].radius[0] << std::endl;
			}
			if (glfwGetKey(window.Get(), GLFW_KEY_M) == GLFW_PRESS)
			{
				pointLights[0].radius += 0.01f;
				std::cout << pointLights[0].radius[0] << std::endl;
			}
		}
		vkDeviceWaitIdle(vulkanInterface->logicalDevice);
		CleanUpVulkan();
	}

	void Core::UpdateUniformBuffers()
	{
		if (renderables.size() == 0) return;
		//std::cout << camera->cameraPos.x << " " << camera->cameraPos.y << " " << camera->cameraPos.z << std::endl;
		cameraBuffer.StartUpdate();
		cameraBuffer.UpdateMap(&inputManager->camera->cameraPos, sizeof(glm::vec3));
		cameraBuffer.StopUpdate(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

		inputManager->camera->view = glm::lookAt(inputManager->camera->cameraPos, inputManager->camera->cameraPos + inputManager->camera->cameraFront, inputManager->camera->cameraUp);
		inputManager->camera->proj = glm::perspective(glm::radians(90.0f), swapChain->GetExtent()->width / (float)swapChain->GetExtent()->height, 0.1f, 10000.0f);
		inputManager->camera->proj[1][1] *= -1; //flipping y coordinate?

		//std::cout << camera->cameraPos.x << " " << camera->cameraPos.y << " " << camera->cameraPos.z << std::endl;

		//for cameraViewMatrix at vert
		cameraViewBuffer.StartUpdate();
		cameraViewBuffer.UpdateMap(&inputManager->camera->view, sizeof(glm::mat4));
		cameraViewBuffer.UpdateMap(&inputManager->camera->proj, sizeof(glm::mat4));
		cameraViewBuffer.StopUpdate(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		//skybox camera
		glm::mat4 model = glm::mat4();
		glm::mat4 newView = glm::lookAt({ 0,0,0 }, glm::vec3(0,0,0) + inputManager->camera->cameraFront, inputManager->camera->cameraUp);
		skyboxCameraBuffer.StartUpdate();
		skyboxCameraBuffer.UpdateMap(&newView, sizeof(glm::mat4));
		skyboxCameraBuffer.UpdateMap(&inputManager->camera->proj, sizeof(glm::mat4));
		skyboxCameraBuffer.UpdateMap(&model, sizeof(glm::mat4));
		skyboxCameraBuffer.StopUpdate(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		//Light updates
		pointLightBuffer.StartUpdate();
		int pointLightSize = (int)pointLights.size();
		pointLightBuffer.UpdateMap(&pointLightSize, sizeof(glm::vec4));
		pointLightBuffer.UpdateMap(pointLights.data(), sizeof(Pointlight)*pointLightSize);
		pointLightBuffer.StopUpdate(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

		pointLights[0].pos = glm::vec4(inputManager->camera->cameraPos, 1);

		//renderables.Get("cube")->Transform = glm::translate(glm::mat4(), glm::vec3(camera->cameraPos.x,camera->cameraPos.y,camera->cameraPos.z));

		for (int i = 0; i < renderables.size(); i++)
		{
			renderables[i]->UpdateUniform();
		}
	}

	void Core::DrawFrame()
	{
		uint32_t imageIndex;

		vkAcquireNextImageKHR(vulkanInterface->logicalDevice, *swapChain->GetSwapchain(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

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

		VkResult result = vkQueueSubmit(vulkanInterface->displayQueue, 1, &submitInfo, VK_NULL_HANDLE);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit");
		}
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphore;


		VkSwapchainKHR swapChains[] = { *swapChain->GetSwapchain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(vulkanInterface->displayQueue, &presentInfo);
	}

	void Core::LoadTextures()
	{
		defaultTexture = new Texture("default.png", vulkanInterface);
		//textures.Push("hill", Texture("MapText.jpg", renderer));
		textures.Push("chalet", Texture("chalet.jpg", vulkanInterface));
		textures.Push("weeb", Texture("Weeb.bmp", vulkanInterface));
		textures.Push("grass", Texture("grasstex.jpg", vulkanInterface));
		textures.Push("cube-uv", Texture("CubeUV.png", vulkanInterface, true));
		textures.Push("sand", Texture("sand.png", vulkanInterface));
		textures.Push("dirt", Texture("dirt.png", vulkanInterface));
		textures.Push("rock", Texture("rock.png", vulkanInterface));
		textures.Push("valley-blend", Texture("valley-blend.png", vulkanInterface));
		textures.Push("back", Texture("back.jpg", vulkanInterface));
		textures.Push("bottom", Texture("bottom.jpg", vulkanInterface));
		textures.Push("front", Texture("front.jpg", vulkanInterface));
		textures.Push("left", Texture("left.jpg", vulkanInterface));
		textures.Push("right", Texture("right.jpg", vulkanInterface));
		textures.Push("top", Texture("top.jpg", vulkanInterface));
		textures.Push("skybox", Texture("cubemap.jpg", vulkanInterface));
	}

	void Core::LoadModels()
	{
		std::vector<Vertex> cubeVertices = 
		{
			{ { -1.0f, 0.0f,-1.0f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } ,{0.0f,1.0f,0.0f}},
			{ { 1.0f, 0.0f,-1.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } ,{0.0f,1.0f,0.0f}},
			{ { 1.0f, 0.0f,1.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f } ,{0.0f,1.0f,0.0f}},
			{ { -1.0f, 0.0f,1.0f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 1.0f } ,{0.0f,1.0f,0.0f}}
		};

		std::vector<uint32_t> cubeIndices = {
		 0,  1,  2,  0,  2,  3,   //front
		 4,  5,  6,  4,  6,  7,   //right
		 8,  9,  10, 8,  10, 11,  //back
		 12, 13, 14, 12, 14, 15,  //left
		 16, 17, 18, 16, 18, 19,  //upper
		 20, 21, 22, 20, 22, 23 }; //bottomk

		models.Push("cube", Model(vulkanInterface, "Cube.obj"));
		models.Push("hill", Model(vulkanInterface, "Hill.obj"));
		models.Push("chalet", Model(vulkanInterface, "chalet.obj"));
		models.Push("bunny", Model(vulkanInterface, "Bunny.obj"));
		models.Push("valley", Model(vulkanInterface, "Valley.obj"));
		//models.Push("skybox", Model(vulkanInterface, cubeVertices, cubeIndices));
		models.Push("skybox", Model(vulkanInterface, "skybox.obj"));
	}

	void Core::CleanUpSwapChain()
	{

		swapChain->CleanUpFrameBuffers();
		vkFreeCommandBuffers(vulkanInterface->logicalDevice, *vulkanInterface->GetCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		vkDestroyPipeline(vulkanInterface->logicalDevice, graphicsPipeline, VK_NULL_HANDLE);
		vkDestroyPipelineLayout(vulkanInterface->logicalDevice, pipelineLayout, VK_NULL_HANDLE);
		vkDestroyPipeline(vulkanInterface->logicalDevice, terrainPipeline, VK_NULL_HANDLE);
		vkDestroyPipelineLayout(vulkanInterface->logicalDevice, terrainPipelineLayout, VK_NULL_HANDLE);
		vkDestroyPipeline(vulkanInterface->logicalDevice, cubemapPipeline, VK_NULL_HANDLE);
		vkDestroyPipelineLayout(vulkanInterface->logicalDevice, cubemapPipelineLayout, VK_NULL_HANDLE);
		vkDestroyRenderPass(vulkanInterface->logicalDevice, renderPass, VK_NULL_HANDLE);

		swapChain->CleanUp();
	}

	void Core::ReCreateSwapChain()
	{
		vkDeviceWaitIdle(vulkanInterface->logicalDevice);
		CleanUpSwapChain();
		swapChain->Create(vulkanInterface);
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateTerrainPipeline();
		CreateCubemapPipeline();
		swapChain->CreateFrameBuffers(&renderPass);
		CreateCommandBuffers();
	}

	void Core::CreateRenderPass()
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

		if (vkCreateRenderPass(vulkanInterface->logicalDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create renderPass");
		}
		else
		{
			std::cout << "Created RenderPass successfully" << std::endl;
		}

	}

	void Core::CreateDescriptorSetlayout()
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

		if (vkCreateDescriptorSetLayout(vulkanInterface->logicalDevice, &layoutInfo, nullptr, &descriptorSetlayout) != VK_SUCCESS)
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
		lb.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		lb.pImmutableSamplers = nullptr;
		setLayoutBinding.push_back(lb);

		VkDescriptorSetLayoutCreateInfo lightInfo = {};
		lightInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		lightInfo.pNext = NULL;
		lightInfo.flags = 0;
		lightInfo.bindingCount = static_cast<uint32_t>(setLayoutBinding.size());
		lightInfo.pBindings = setLayoutBinding.data();


		if (vkCreateDescriptorSetLayout(vulkanInterface->logicalDevice, &lightInfo, nullptr, &lightDescriptorSetlayout) != VK_SUCCESS)
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

		if (vkCreateDescriptorSetLayout(vulkanInterface->logicalDevice, &cameraLayoutInfo, nullptr, &cameraSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create light descriptor layout");
		}

	}

	void Core::CreateTerrainDescriptorSetlayout()
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
		samplerLayoutBinding.descriptorCount = 5;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; //fragment shaderille

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(vulkanInterface->logicalDevice, &layoutInfo, nullptr, &terrainDescriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create DescriptorLayout");
		}
		else
		{
			std::cout << "Created Terrain DescriptorSetLayout" << std::endl;
		}
	}

	VkShaderModule Core::CreateShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();

		std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
		memcpy(codeAligned.data(), code.data(), code.size());
		createInfo.pCode = codeAligned.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(vulkanInterface->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
		}
		else
		{
			std::cout << "Shader module created successfully" << std::endl;
		}
		return shaderModule;
	}

	VkShaderModule Core::CreateShaderModule(const std::vector<unsigned int>& code)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();

		std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
		memcpy(codeAligned.data(), code.data(), code.size());
		createInfo.pCode = codeAligned.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(vulkanInterface->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
		}
		else
		{
			std::cout << "Shader module created successfully" << std::endl;
		}
		return shaderModule;
	}

	void Core::CreateGraphicsPipeline()
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

		if (vkCreatePipelineLayout(vulkanInterface->logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
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
//TODO: [ UNASSIGNED-CoreValidation-Shader-DescriptorTypeMismatch ] Object: VK_NULL_HANDLE (Type = 0) | Shader expects at least 12 descriptors for binding 0.1 but only 1 provided
		if (vkCreateGraphicsPipelines(vulkanInterface->logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphicspipeline");
		}
		else
		{
			std::cout << "Created graphics pipeline successfully" << std::endl;
		}
		vkDestroyShaderModule(vulkanInterface->logicalDevice, vertShaderModule, VK_NULL_HANDLE);
		vkDestroyShaderModule(vulkanInterface->logicalDevice, fragShaderModule, VK_NULL_HANDLE);
	}

	void Core::CreateTerrainPipeline()
	{
		auto fragShaderCode = ReadFile("shaders/terrainfrag.spv");
		auto vertShaderCode = ReadFile("shaders/terrainvert.spv");

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
		std::array<VkDescriptorSetLayout, 3> setLayouts = { terrainDescriptorSetLayout,lightDescriptorSetlayout,cameraSetLayout };
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

		if (vkCreatePipelineLayout(vulkanInterface->logicalDevice, &pipelineLayoutInfo, nullptr, &terrainPipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}
		else
		{
			std::cout << "Created terrain pipelinelayout" << std::endl;
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
		pipelineCreateInfo.layout = terrainPipelineLayout;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.pDepthStencilState = &depthStencil;
		pipelineCreateInfo.subpass = 0;

		VkResult result = vkCreateGraphicsPipelines(vulkanInterface->logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &terrainPipeline);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphicspipeline");
		}
		else
		{
			std::cout << "Created graphics pipeline successfully" << std::endl;
		}
		vkDestroyShaderModule(vulkanInterface->logicalDevice, vertShaderModule, VK_NULL_HANDLE);
		vkDestroyShaderModule(vulkanInterface->logicalDevice, fragShaderModule, VK_NULL_HANDLE);
	}

	void Core::CreateCubemapPipeline()
	{
		auto fragShaderCode = ReadFile("shaders/skyboxfrag.spv"); //cubemap shaders
		auto vertShaderCode = ReadFile("shaders/skyboxvert.spv");

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
		auto bindingDescription = Vertex::getCubemapBindingDescription();
		auto attributeDescription = Vertex::getCubemapAttributeDescription();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

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
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //VK_CULL_MODE_FRONT_BIT;
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
		std::array<VkDescriptorSetLayout, 1> setLayouts = { descriptorSetlayout };
		pipelineLayoutInfo.setLayoutCount = static_cast<int>(setLayouts.size());
		pipelineLayoutInfo.pSetLayouts = setLayouts.data();

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_FALSE; //False
		depthStencil.depthWriteEnable = VK_FALSE; //false
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};

		if (vkCreatePipelineLayout(vulkanInterface->logicalDevice, &pipelineLayoutInfo, nullptr, &cubemapPipelineLayout) != VK_SUCCESS)
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

		if (vkCreateGraphicsPipelines(vulkanInterface->logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &cubemapPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphicspipeline");
		}
		else
		{
			std::cout << "Created graphics pipeline successfully" << std::endl;
		}
		vkDestroyShaderModule(vulkanInterface->logicalDevice, vertShaderModule, VK_NULL_HANDLE);
		vkDestroyShaderModule(vulkanInterface->logicalDevice, fragShaderModule, VK_NULL_HANDLE);
	}

	void Core::CreateTextureSampler()
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

		if (vkCreateSampler(vulkanInterface->logicalDevice, &samplerInfo, nullptr, &defaultSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	void Core::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 3> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 4 + (int)renderables.size() + (int)terrains.size();
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 10 + (int)renderables.size() + (int)terrains.size();
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount = 100 + 200;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(renderables.size() * 2 + terrains.size() * 2 + 200);

		if (vkCreateDescriptorPool(vulkanInterface->logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Descriptor Pool");
		}
		else
		{
			std::cout << "Created Descriptor Pool Successfully" << std::endl;
		}
	}

	void Core::CreateLights()
	{
		for (int i = 0; i < 1; i++) {
			// do { color = { glm::linearRand(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)) }; } while (color.length() < 0.8f);

			pointLights.emplace_back(glm::linearRand(glm::vec3(1, 1, 0), glm::vec3(1, 1, 0)), 0.1f, 0.001f, glm::vec4(1, 1, 1, 1));
		}

		int lightnum = (int)pointLights.size();

		VkDeviceSize size = sizeof(Pointlight) * lightnum + sizeof(glm::vec4);

		pointLightBuffer = Buffer(vulkanInterface);
		pointLightBuffer.StartMapping(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pointLightBuffer.Map(&lightnum, sizeof(glm::vec4));
		pointLightBuffer.Map(pointLights.data(), sizeof(Pointlight)*pointLights.size());
		pointLightBuffer.StopMapping();

	}

	void Core::CreateLightDescriptorSets()
	{
		VkDescriptorSetLayout layouts[] = { lightDescriptorSetlayout };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		if (vkAllocateDescriptorSets(vulkanInterface->logicalDevice, &allocInfo, &lightDescriptor) != VK_SUCCESS)
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

		vkUpdateDescriptorSets(vulkanInterface->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void Core::CreateCameraDescriptorSets()
	{

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &cameraSetLayout;

		VkResult result = vkAllocateDescriptorSets(vulkanInterface->logicalDevice, &allocInfo, &cameraSet);

		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptorset");
		}
		else
		{
			std::cout << "Allocated descriptor succesfully" << std::endl;
		}

		//for cameraposition at frag
		cameraBuffer = Buffer(vulkanInterface);
		cameraBuffer.StartMapping(sizeof(glm::vec3), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		cameraBuffer.Map(&inputManager->camera->cameraPos, sizeof(glm::vec3));
		cameraBuffer.StopMapping();

		VkDescriptorBufferInfo cameraBufferInfo = {};
		cameraBufferInfo.buffer = *cameraBuffer.GetBuffer();
		cameraBufferInfo.offset = 0;
		cameraBufferInfo.range = cameraBuffer.bufferSize;

		inputManager->camera->view = glm::lookAt(inputManager->camera->cameraPos, inputManager->camera->cameraPos + inputManager->camera->cameraFront, inputManager->camera->cameraUp);
		inputManager->camera->proj = glm::perspective(glm::radians(90.0f), swapChain->GetExtent()->width / (float)swapChain->GetExtent()->height, 0.1f, 10000.0f);
		inputManager->camera->proj[1][1] *= -1; //flipping y coordinate?


		//for cameraViewMatrix at vert
		cameraViewBuffer = Buffer(vulkanInterface);
		cameraViewBuffer.StartMapping(sizeof(glm::mat4) * 2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		cameraViewBuffer.Map(&inputManager->camera->view, sizeof(glm::mat4));
		cameraViewBuffer.Map(&inputManager->camera->proj, sizeof(glm::mat4));
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

		vkUpdateDescriptorSets(vulkanInterface->logicalDevice, (uint32_t)camerawrites.size(), camerawrites.data(), 0, nullptr);
	}

	void Core::CreateDescriptorSets()
	{
		//For our cubemap
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetlayout;

		VkResult result = vkAllocateDescriptorSets(vulkanInterface->logicalDevice, &allocInfo, &cubemapDescriptorSet); //Creating DS for cubemap
		if (result != VK_SUCCESS) { throw std::runtime_error("Failed to allocate descriptorset"); }
		else { std::cout << "Allocated descriptor succesfully" << std::endl; }
			
		inputManager->camera->view = glm::lookAt(inputManager->camera->cameraPos, inputManager->camera->cameraPos + inputManager->camera->cameraFront, inputManager->camera->cameraUp);
		inputManager->camera->proj = glm::perspective(glm::radians(90.0f), swapChain->GetExtent()->width / (float)swapChain->GetExtent()->height, 0.1f, 10000.0f);
		inputManager->camera->proj[1][1] *= -1; //flipping y coordinate?
		inputManager->camera->view = glm::translate(inputManager->camera->view, { 0,0,0 });

		glm::mat4 model = glm::mat4();
		skyboxCameraBuffer = Buffer(vulkanInterface);
		skyboxCameraBuffer.StartMapping(sizeof(glm::mat4) * 3, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		skyboxCameraBuffer.Map(&inputManager->camera->view, sizeof(glm::mat4));
		skyboxCameraBuffer.Map(&inputManager->camera->proj, sizeof(glm::mat4));
		skyboxCameraBuffer.Map(&model, sizeof(glm::mat4));
		skyboxCameraBuffer.StopMapping();

		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {}; 
		VkDescriptorBufferInfo bufferInfo = {};
		//bufferInfo.buffer = *renderables.Get("skybox")->GetUniformBuffer();
		bufferInfo.buffer = *skyboxCameraBuffer.GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(glm::mat4);

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = cubemapDescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		VkDescriptorImageInfo imageInfo = { };
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = cubemappi->imageView;
		imageInfo.sampler = defaultSampler;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = cubemapDescriptorSet;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(vulkanInterface->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		//this is for our 3D objects excluding terrain
		for (int i = 0; i < renderables.size(); i++)
		{

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &descriptorSetlayout;

			result = vkAllocateDescriptorSets(vulkanInterface->logicalDevice, &allocInfo, renderables.Get(i)->GetDescriptorSet());
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

				vkUpdateDescriptorSets(vulkanInterface->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

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

				vkUpdateDescriptorSets(vulkanInterface->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}
	}

	void Core::CreateTerrainDescriptorSets()
	{
		for (int i = 0; i < terrains.size(); i++)
		{
			VkDescriptorSetLayout layouts[] = { terrainDescriptorSetLayout };

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = layouts;

			VkResult result = vkAllocateDescriptorSets(vulkanInterface->logicalDevice, &allocInfo, terrains[i].GetDescriptorSet());
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

				vkUpdateDescriptorSets(vulkanInterface->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

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
				descriptorWrites[1].descriptorCount = 5;
				descriptorWrites[1].pImageInfo = imageInfos.data();

				vkUpdateDescriptorSets(vulkanInterface->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

				return;
			}
		}
	}

	void Core::CreateCommandBuffers()
	{
		commandBuffers.resize(swapChain->GetFramebufferSize());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = *vulkanInterface->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(vulkanInterface->logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command buffers");
		}
		else
		{
			std::cout << "Created command buffers successfully" << std::endl;
		}

		for (int i = 0; i < commandBuffers.size(); i++)
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

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, cubemapPipeline);

			VkDeviceSize offsets[] = { 0 };

			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, cubemappi->cubeBox->GetVertexBuffer()->GetBuffer(),offsets);
			vkCmdBindIndexBuffer(commandBuffers[i], *cubemappi->cubeBox->GetIndexBuffer()->GetBuffer(),0,VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, cubemapPipelineLayout, 0, 1, &cubemapDescriptorSet, 0, nullptr); 
			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(cubemappi->cubeBox->GetVertexBuffer()->GetVerticesSize()), 1, 0, 0, 0);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			for (int j = 0; j < renderables.size(); j++)
			{
				VkBuffer vertexBuffers[] = { *renderables[j]->GetVertexBuffer() };
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
				VkBuffer indexBuffer = *renderables[j]->GetIndexBuffer();
				VkDeviceSize indexOffsets[] = { 0 };
				vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
				std::array<VkDescriptorSet, 3> descriptor_sets = { *renderables[j]->GetDescriptorSet(), lightDescriptor, cameraSet };
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, (uint32_t)descriptor_sets.size(), descriptor_sets.data(), 0, nullptr);

				vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(renderables[j]->GetIndicesSize()), 1, 0, 0, 0);
			}

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipeline);

			for (int j = 0; j < terrains.size(); j++)
			{
				VkBuffer vertexBuffer[] = { *terrains[j].GetVertexBuffer() };
				VkDeviceSize offsets[] = { 0 };

				VkBuffer indexBuffer = *terrains[j].GetIndexBuffer();
				VkDescriptorSet descriptorSets[] = { *terrains[j].GetDescriptorSet() , lightDescriptor, cameraSet };

				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffer, offsets);
				vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipelineLayout, 0, 3, descriptorSets, 0, nullptr);

				vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(terrains[j].GetIndicesSize()), 1, 0, 0, 0);
			}

			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to end command buffer");
			}
		}
	}

	void Core::CreateSemaphores()
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(vulkanInterface->logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(vulkanInterface->logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create semaphore");
		}
		else
		{
			std::cout << "Created semaphores successfully" << std::endl;
		}
	}

	void Core::CleanUpVulkan()
	{
		CleanUpSwapChain();
		defaultTexture->CleanUp();

		textures.CleanUp();
		models.CleanUp();

		vkDestroySampler(vulkanInterface->logicalDevice, defaultSampler, nullptr);
		vkDestroyDescriptorPool(vulkanInterface->logicalDevice, descriptorPool, nullptr);

		vkDestroyDescriptorSetLayout(vulkanInterface->logicalDevice, descriptorSetlayout, nullptr);
		vkDestroyDescriptorSetLayout(vulkanInterface->logicalDevice, lightDescriptorSetlayout, nullptr);
		vkDestroyDescriptorSetLayout(vulkanInterface->logicalDevice, cameraSetLayout, nullptr);

		if (renderables.size() != 0)
		{
			cameraBuffer.CleanUpBuffer();
			cameraViewBuffer.CleanUpBuffer();
			pointLightBuffer.CleanUpBuffer();
		}
		renderables.CleanUp();

		vkDestroySemaphore(vulkanInterface->logicalDevice, renderFinishedSemaphore, VK_NULL_HANDLE);
		vkDestroySemaphore(vulkanInterface->logicalDevice, imageAvailableSemaphore, VK_NULL_HANDLE);
		vulkanInterface->CleanUp(); //Destroys device, surface and instance; CleanCommandpools
		glfwDestroyWindow(window.Get());
		glfwTerminate();
	}
}
