#include <cstdlib>
#include <vector>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <cstring>
#include <iostream>

#include "Application.hpp"

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
								this->debugMessenger, nullptr);
	}

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

	// TODO 19/03/2023: This is already handled by VkLayers now. Make sure to remove in the next commit.
	// - - -
	// Count how many extensions we have and put them into an array
	// uint32_t extensionCount = 0;
	// vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	// std::vector<VkExtensionProperties> extensionsVec(extensionCount);
	// vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionsVec.data());

	// // List vulkan available extensions
	// // std::cout << "Available extensions:\n";
	// // for (const auto &extension : extensionsVec) {
	// //     std::cout << '\t' << extension.extensionName << '\n';
	// // }

	// // std::cout << "Avaadwfafilable extensions:\n";

	// if (glfwExtensionCount == 0) {
	// 	std::cout << "Error:" << '\n';
	// }

	// // Check if all glfw extensions are supported by Vulkan
	// for (int i = 0; i < glfwExtensionCount - 1; i++) {
	// 	const char *curExtension = glfwExtensions.at(0);

	// 	bool foundExtension = false;
	// 	for (const auto &extension : extensionsVec) {
	// 		if (strcmp(curExtension, extension.extensionName))
	// 			foundExtension = true;
	// 	}

	// 	if (!foundExtension) {
	// 		// TODO: Redo this message
	// 		std::cout << "Error: The glfw extension '" << curExtension 
	// 			<< "' couldn't been supported by Vulkan.\n";
	// 	}
	// 	// std::cout << '\t' << *curExtension << '\n';
	// }
	// - - -
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
		(this->m_vkInstance, &dbCreateInfo, nullptr, &debugMessenger) 
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
