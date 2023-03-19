#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

class Application
{
private:
	VkInstance m_vkInstance;
	const std::vector<const char*> m_validationLayers;
	const bool ENABLE_VALIDATION_LAYERS;
	VkDebugUtilsMessengerEXT debugMessenger;

public:
	Application();
	~Application();
	void vkCreateInfo();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();

	// ------------------ Vulkan Messenger Debugger setup ------------------	
	void setupDebugMessenger();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& dbCreateInfo);
	void destroyDebugUtilsMessengerEXT(
		VkInstance vkInstance, 
		VkDebugUtilsMessengerEXT dbMessenger, 
		const VkAllocationCallbacks* pAllocator);
};
