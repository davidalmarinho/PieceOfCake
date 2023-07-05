#pragma once

#include <vulkan/vulkan.h>

class VulkanDebugger
{
private:
  VkDebugUtilsMessengerEXT debugMessenger;

  void setupDebugMessenger(VkInstance vkInstance);

public:
#ifdef NDEBUG
  static const bool ENABLE_VALIDATION_LAYERS{false};
#else
  static const bool ENABLE_VALIDATION_LAYERS{true};
#endif


  VulkanDebugger(VkInstance vkInstance);
  ~VulkanDebugger();

  VkResult createDebugUtilsMessengerEXT(VkInstance vkInstance,
                                        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator,
                                        VkDebugUtilsMessengerEXT *pDebugMessenger);

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
      void *pUserData);

  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

  void destroyDebugUtilsMessengerEXT(
    VkInstance vkInstance,
    const VkAllocationCallbacks *pAllocator);
};