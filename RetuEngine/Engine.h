#pragma once
#include "VDeleter.h"
#include <fstream>
#include <vector>
#include <cstring>
/*Graphics Related Headeds*/
#include "Window.h"
#include "SwapChain.h"
#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"
#include "QueueFamily.h"
#include "CommandPool.h"
/*Engine Objects*/
#include "Gameobject.h"

namespace RetuEngine 
{
	static std::vector<char> ReadFile(const std::string& filename)
	{
		std::ifstream file(filename,std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			throw std::runtime_error("Cannot open file");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	class Engine
	{
	public:
		Engine();
		~Engine();

		void InitVulkan();
		void CleanUpVulkan();
		void GameLoop();
		Window windowObj;

	private:
		void InitWindow();
		void ReCreateSwapChain();
		static void OnWindowResized(GLFWwindow* window,int width, int height)
		{
			if (width == 0 || height == 0)
			{
				return;
			}
			Engine* engine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
			engine->ReCreateSwapChain();
		}
		void CreateInstance();
		void CleanUpSwapChain();
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void SetupDebugCallback();
		void CreateSurface();
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		void GetPhysicalDevices();
		int RateDeviceSuitability(VkPhysicalDevice deviceToRate);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		void CreateLogicalDevice();
		void CreateRenderPass();
		void CreateDescriptorSetlayout();
		void CreateDescriptorPool();
		void CreateDescriptorSet();
		void CreateGraphicsPipeline();
		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		void CreateCommandBuffers();
		void CreateSemaphores();
		void DrawFrame();


		VkResult CreateDebugReportReportCallbackEXT(
			VkInstance istance,
			const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugReportCallbackEXT* pCallback);

		static void DestoyDebugReportCallbackEXT(
			VkInstance instance,
			VkDebugReportCallbackEXT callback,
			const VkAllocationCallbacks* pAllocator
			);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t obj,
			size_t location,
			int32_t code,
			const char* layerPrefix,
			const char* msg,
			void* userData
		)
		{
			std::cerr << "Validation layer: " << msg << std::endl;
			return VK_FALSE;
		};

		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		//Vulkan handles
		VkInstance instance;
		VkDebugReportCallbackEXT callback;
		VkSurfaceKHR surface;
		VkDevice logicalDevice;
		const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkQueue displayQueue;
		VkQueue transferQueue;

		SwapChain* swapChain;

		VkRenderPass renderPass;

		VkDescriptorSetLayout descriptorSetlayout;

		VkPipelineLayout pipelineLayout;

		VkPipeline graphicsPipeline;

		CommandPool* graphicsCommandPool;
		CommandPool* transferCommandPool;
		VkDescriptorPool descriptorPool;

		VertexBuffer* vertexBuffer;
		IndexBuffer* indexBuffer;
		UniformBuffer* uniformBuffer;

		std::vector<VkCommandBuffer> commandBuffers;

		VkSemaphore imageAvailableSemaphore;

		VkSemaphore renderFinishedSemaphore;

		VkDescriptorSet descriptorSet;

		Gameobject* mainRectangle;

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif // NDEBUG
	};
}
