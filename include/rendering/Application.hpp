#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <optional>

class Application
{
private:
	VkInstance m_vkInstance;
	const std::vector<const char*> m_validationLayers;
	const bool ENABLE_VALIDATION_LAYERS;

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	VkSurfaceKHR m_vkSurface;

	const std::vector<const char*> m_deviceExtensions;

	VkDebugUtilsMessengerEXT m_debugMessenger;
	
	VkPipelineLayout m_pipelineLayout;

	VkPipeline m_graphicsPipeline;

public:
	Application();
	~Application();
	void vkCreateInfo();
	void createGraphicsPipeline(VkRenderPass renderPass);
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	
	// ------------------ Vulkan Physical devices setup ------------------	
	void pickPhysicalDevice();
	
	/**
	 * Check if a graphics card can handle with all Vulkan operations
	 *
	 * @return True if the graphics card can handle with all operations that vulkan
	 *		   needs, otherwise returns False.
	 */
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
	void createLogicalDevice();

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

	// Swapchain
	bool checkDeviceExtensionSupport(VkPhysicalDevice vkDevice);

	// ------------------ Shaders modules setup ------------------
	VkShaderModule createShaderModule(const std::vector<char>& code);

	// Getters and setters
	VkInstance getVkInstance();
	VkPhysicalDevice getPhysicalDevice();
	VkDevice getDevice();
	VkSurfaceKHR getVkSurface();
	VkSurfaceKHR* getVkSurfacePtr();
	VkPipelineLayout getPipelineLayout();
	VkPipeline getGraphicsPipeline();
	VkQueue getGraphicsQueue();
	VkQueue getPresentQueue();
};

