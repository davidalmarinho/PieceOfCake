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
#include "Texture.hpp"

#include "Model.hpp"
#include "ECS.hpp"
#include "ModelRenderer.hpp"

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

  enum class MsaaSetting
  {
    MSAA64X = 64,
    MSAA32X = 32,
    MSAA16X = 16,
    MSAA8X = 8,
    MSAA4X = 4,
    MSAA2X = 2,
    DISABLED = 1,
  };

  friend MsaaSetting operator++(MsaaSetting& msaaSetting, int);
  friend std::ostream& operator<<(std::ostream& os, MsaaSetting mssaSetting);

  // Graphics settings configuration.
  MipmapSetting mipmapSetting;
  // Multisampling cnfiguration --MSAA
  MsaaSetting msaaSetting;

  bool sampleShading = true;

  Renderer();
  ~Renderer();

  void clean();
  void restart();

  void init();
  void initRendering();
  void drawFrame();

  // Getters and Setters

  VkDevice getDevice();
  VkPhysicalDevice getPhysicalDevice();
  VkCommandPool getCommandPool();
  VkQueue getGraphicsQueue();
  const std::unique_ptr<SwapChain> &getSwapChain() const;

  VkSampleCountFlagBits getMsaaSample();
  VkSampleCountFlagBits getMaxMsaaSamples();
  const std::optional<std::reference_wrapper<Entity>> getEntity(int index);

  void addEntity(Entity& e);
  
private:
  VkSurfaceKHR surface;
  std::unique_ptr<VulkanDebugger> vulkanDebugger;
  std::unique_ptr<SwapChain> swapChain;
  std::vector<std::unique_ptr<Pipeline>> pipelines;
  std::vector<std::reference_wrapper<Entity>> entitiesVec;

  VkDevice device;
  VkInstance vkInstance;

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  // Command Pool
  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers; // Allocates command buffers.

  // Multisampling configuration --MSAA
  VkSampleCountFlagBits msaaSamples    = VK_SAMPLE_COUNT_1_BIT;
  VkSampleCountFlagBits maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT; // Keeps track of how many samples the hardware can use.
  VkSampleCountFlagBits getMaxUsableSampleCount();

  void initVulkan();

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
};