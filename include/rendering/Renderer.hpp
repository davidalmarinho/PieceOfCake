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
#include "Model.hpp"
#include "VulkanDebugger.hpp"
#include "AssetPool.hpp"
#include "KeyListener.hpp"
#include "Pipeline.hpp"
#include "QueueFamilyIndices.hpp"
#include "SwapChain.hpp"
#include "Texture.hpp"

// Frames which should be processed concurrently.
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

class Renderer
{
public:
  enum class MipmapSetting
  {
    DISABLED = 0,
    LINEAR   = 1,
    NEAREST  = 2 // TODO: Implement this.
  };

  friend MipmapSetting operator++(MipmapSetting& mipmapSetting, int);
  friend std::ostream& operator<<(std::ostream& os, MipmapSetting mipmapSetting);

  Renderer();
  ~Renderer();

  void clean();
  void restart();

  // Graphics settings configuration.
  MipmapSetting mipmapSetting;

  void init();
  void drawFrame();

  // Getters and Setters

  VkDevice getDevice();
  VkPhysicalDevice getPhysicalDevice();
  VkCommandPool getCommandPool();
  VkQueue getGraphicsQueue();
  const std::unique_ptr<SwapChain> &getSwapChain() const;
  const std::unique_ptr<Pipeline> &getPipeline() const;

private:
  void initVulkan();
  VkSurfaceKHR surface;
  std::unique_ptr<Model> model;
  std::unique_ptr<VulkanDebugger> vulkanDebugger;
  std::unique_ptr<SwapChain> swapChain;
  std::unique_ptr<Pipeline> pipeline;

  VkDevice device;
  VkInstance vkInstance;

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  // Command Pool
  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers; // Allocates command buffers.

  void createInstance();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void recreateSwapChain();
  void createCommandPool();
  void createCommandBuffers();
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  bool isDeviceSuitable(VkPhysicalDevice device);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();
  void loadModels();
};