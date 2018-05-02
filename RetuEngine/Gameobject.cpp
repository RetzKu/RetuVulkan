#include "Gameobject.h"



namespace RetuEngine
{
	Gameobject::Gameobject(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, const VkCommandPool* commandPool, const VkQueue* queue)
	{
		this->logicalDevice = logicalDevice;
	}


	Gameobject::~Gameobject()
	{
	}

}