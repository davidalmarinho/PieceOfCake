#include <vector>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string.h>
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
	vkDestroyInstance(this->m_vkInstance, nullptr);
}

void Application::vkCreateInfo()
{
	if (this->ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
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
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = 
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	createInfo.enabledExtensionCount   = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	if (this->ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(this->m_validationLayers.size());
		createInfo.ppEnabledLayerNames = this->m_validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	// Create the instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_vkInstance);
	
	// Check if vkInstance could be created
	if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS) {
		throw std::runtime_error("Error: Couldn't create VkInstance");
	}

	// Count how many extensions we have and put them into an array
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensionsVec(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionsVec.data());

	// List vulkan available extensions
	// std::cout << "Available extensions:\n";
	// for (const auto &extension : extensionsVec) {
	//     std::cout << '\t' << extension.extensionName << '\n';
	// }

	// std::cout << "Avaadwfafilable extensions:\n";

	// Check if all glfw extensions are supported by Vulkan
	for (int i = 0; i < glfwExtensionCount - 1; i++) {
		const char *curExtension = *glfwExtensions;

		bool foundExtension = false;
		for (const auto &extension : extensionsVec) {
			if (strcmp(curExtension, extension.extensionName))
				foundExtension = true;
		}

		if (!foundExtension) {
			// TODO: Redo this message
			std::cout << "Error: The glfw extension '" << curExtension 
				<< "' couldn't been supported by Vulkan.\n";
		}
		// std::cout << '\t' << *curExtension << '\n';
	}
}

bool Application::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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
