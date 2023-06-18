#include <iostream>

#include "VulkanDebugger.hpp"
#include "vulkan/vulkan.h"

VulkanDebugger::VulkanDebugger(VkInstance vkInstance)
{
  this->setupDebugMessenger(vkInstance);
}

VulkanDebugger::~VulkanDebugger()
{

}

// Vulkan Messenger Debugger setup
VkResult VulkanDebugger::createDebugUtilsMessengerEXT(VkInstance vkInstance, 
      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
      const VkAllocationCallbacks* pAllocator, 
      VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) 
        vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
        return func(vkInstance, pCreateInfo, pAllocator, pDebugMessenger);
    } 
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugger::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) 
{	
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;	
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		// TODO: Message is important enough to show
	}	
	return VK_FALSE;
}

void VulkanDebugger::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& dbCreateInfo)
{
  dbCreateInfo = {};
	dbCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	dbCreateInfo.messageSeverity = 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	dbCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	
	dbCreateInfo.pfnUserCallback = this->debugCallback;
	// dbCreateInfo.pUserData = nullptr;
}

void VulkanDebugger::setupDebugMessenger(VkInstance vkInstance) 
{
  if (!VulkanDebugger::ENABLE_VALIDATION_LAYERS) return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  if (this->createDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to set up debug messenger.\n");
  }
}

void VulkanDebugger::destroyDebugUtilsMessengerEXT(
  VkInstance vkInstance, 
  const VkAllocationCallbacks* pAllocator) 
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) 
          vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(vkInstance, this->debugMessenger, pAllocator);
    }
}