#include <cstdint>
#include <cstdlib>
#include <vector>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <vulkan/vulkan_core.h>
#include <set>

#include "Application.hpp"
#include "FamilyQueues.hpp"

#ifdef NDEBUG
Application::Application() : m_validationLayers(1, "VK_LAYER_KHRONOS_validation"), ENABLE_VALIDATION_LAYERS(false)
{

}
#else
Application::Application() : m_validationLayers(1, "VK_LAYER_KHRONOS_validation"), ENABLE_VALIDATION_LAYERS(true)
{

}
#endif

Application::~Application()
{
	if (this->ENABLE_VALIDATION_LAYERS) {
		destroyDebugUtilsMessengerEXT(this->m_vkInstance, 
								this->m_debugMessenger, nullptr);
	}

	vkDestroyDevice(this->m_device, nullptr);
	vkDestroySurfaceKHR(this->m_vkInstance, this->m_vkSurface, nullptr);
	vkDestroyInstance(this->m_vkInstance, nullptr);
}

void Application::vkCreateInfo()
{
	if (this->ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
		throw std::runtime_error("Validation layers requested, but not available!\n");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = "Triangle in Vulkan :)";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName        = "PieceOfCake";
	appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion         = VK_API_VERSION_1_0;
	
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Connect Vulkan to Glfw
	auto glfwExtensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
	uint32_t glfwExtensionCount = glfwExtensions.size();
	createInfo.ppEnabledExtensionNames = glfwExtensions.data();

	VkDebugUtilsMessengerCreateInfoEXT dbCreateInfo{};
	// If validation layers are enabled we want to include them
	if (this->ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(this->m_validationLayers.size());
		createInfo.ppEnabledLayerNames = this->m_validationLayers.data();

		populateDebugMessengerCreateInfo(dbCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &dbCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// Check if the instance can be created and actully create the instance
	if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS) {
		throw std::runtime_error("Error: Couldn't create VkInstance");
	}
}

bool Application::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Check if the required layers are available
	for (const char* layerName : this->m_validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			// std::cout << "Available Layer: " << layerName << "\n";
			// std::cout << "Required Layer: " << layerProperties.layerName << "\n";

			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}

// ------------------ Physical devices setup ------------------
void Application::pickPhysicalDevice()
{
	// List graphics card
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->m_vkInstance, &deviceCount, nullptr);

	// Throw error if we haven't find graphics card
	if (deviceCount == 0) {
		throw std::runtime_error("Error: Failed to find GPUs with Vulkan support.\n");
	}

	std::vector<VkPhysicalDevice> devicesVec(deviceCount);
	vkEnumeratePhysicalDevices(this->m_vkInstance, &deviceCount, devicesVec.data());

	// Check if the computer has a graphics card that Vulkan can handle all
	// the operations that it needs.
	for (const auto& device : devicesVec) {
		if (isDeviceSuitable(device)) {
			this->m_physicalDevice = device;
			break;
		}
	}
	if (this->m_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("Error: Failed to find a suitable GPU.\n");
	}

}

bool Application::isDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	// Get device properties so we can check if the graphics cards 
	// support geometry shaders, in this case
	/*
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(this->m_physicalDevice, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(this->m_physicalDevice, &deviceFeatures);

	return deviceProperties.deviceType == 
		VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
	*/

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, this->m_vkSurface);

	return indices.isComplete();
}


void Application::createLogicalDevice()
{
	// Specifies the number of queues we want for a single queue family.
	// We gonna specify it just to be a queue which supports graphics capabilities
	QueueFamilyIndices indices = findQueueFamilies(this->m_physicalDevice, this->m_vkSurface);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoVec;
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	// Creating queues
	// TODO: In future is a good idea to make this multithread
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount       = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfoVec.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	// Creating Logical Device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	// Pointers to the queue creation info and device features structs
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoVec.size());
	createInfo.pQueueCreateInfos    = queueCreateInfoVec.data();
	createInfo.pEnabledFeatures     = &deviceFeatures;

	// Guarantee compatibility with older devices and older vulkan devices.
	// Because this isn't needed anymore.
	createInfo.enabledExtensionCount = 0;
	if (this->ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount = 
			static_cast<uint32_t>(this->m_validationLayers.size());
			createInfo.ppEnabledLayerNames = this->m_validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	
	// TODO: Here we can specify extensions to do more cool stuff with vulkan
	
	// Instantiate the logical
	if (vkCreateDevice(this->m_physicalDevice, &createInfo, 
					nullptr, &this->m_device) != VK_SUCCESS) {
		throw std::runtime_error("Error: Logical Device creation has failed!\n");
	}

	// Retrieve queue handles for each queue family
	vkGetDeviceQueue(this->m_device, indices.presentFamily.value(), 0,
			   &this->m_presentQueue);
}

// ------------------ Vulkan Messenger Debugger setup ------------------

VKAPI_ATTR VkBool32 VKAPI_CALL Application::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		// TODO: Message is important enough to show
	}

	return VK_FALSE;
}

VkResult createDebugUtilsMessengerEXT(VkInstance vkInstance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
	
	if (func != nullptr)
		return func(vkInstance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Application::setupDebugMessenger()
{
	if (!this->ENABLE_VALIDATION_LAYERS) return;

	// Setup messenger's callbacks
	VkDebugUtilsMessengerCreateInfoEXT dbCreateInfo{};
	populateDebugMessengerCreateInfo(dbCreateInfo);

	if (createDebugUtilsMessengerEXT
		(this->m_vkInstance, &dbCreateInfo, nullptr, &this->m_debugMessenger) 
		    != VK_SUCCESS) {
		throw std::runtime_error("Error: Failed to set up debug messenger.\n");
	}
}

void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& dbCreateInfo)
{
	dbCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	dbCreateInfo.messageSeverity = 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	dbCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	
	dbCreateInfo.pfnUserCallback = Application::debugCallback;
	// dbCreateInfo.pUserData = nullptr;
}

/**
 * @brief Get extensions which are required by vulkan, like:
 *		  -> VK_KHR_surface
 *
 * @return 
 */
std::vector<const char*> Application::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	std::vector<const char*> extensionsVec(
			glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (this->ENABLE_VALIDATION_LAYERS) {
		extensionsVec.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensionsVec;
}

void Application::destroyDebugUtilsMessengerEXT(
	VkInstance vkInstance, 
	VkDebugUtilsMessengerEXT dbMessenger, 
	const VkAllocationCallbacks *pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
				vkGetInstanceProcAddr(this->m_vkInstance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(vkInstance, dbMessenger, pAllocator);
	}
}

// Getters and setters
VkInstance Application::getVkInstance()
{
	return this->m_vkInstance;
}

VkSurfaceKHR* Application::getVkSurfacePtr()
{
	return &this->m_vkSurface;
}

