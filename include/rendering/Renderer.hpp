#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <memory>
#include <glm/glm.hpp>
#include <array>

#include "Window.hpp"
#include "VulkanDebugger.hpp"
#include "AssetPool.hpp"
#include "KeyListener.hpp"
#include "Pipeline.hpp"
#include "QueueFamilyIndices.hpp"
#include "SwapChain.hpp"

// Frames which should be processed concurrently.
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

class Renderer
{
public:
  void run();
  void initVulkan();
  void mainLoop();
  void cleanup();
  void createInstance();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void recreateSwapChain();
  void createCommandPool();
  void createCommandBuffers();
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void drawFrame();
  bool isDeviceSuitable(VkPhysicalDevice device);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();

private:
  std::unique_ptr<Window> window;
  std::unique_ptr<VulkanDebugger> vulkanDebugger;
  std::unique_ptr<SwapChain> swapChain;

  // TODO: Put these in another file
  VkInstance vkInstance;
  VkSurfaceKHR surface;

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  // Command Pool
  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers; // Allocates command buffers.

  std::unique_ptr<Pipeline> pipeline;
};