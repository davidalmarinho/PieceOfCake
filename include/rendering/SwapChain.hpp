#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>

struct SwapChainSupportDetails
{
  // We need to check for 3 kinds of prorperties:
  // • Basic surface capabilities (min/max number of images in swap chain,
  //   min/-max width and height of images)
  VkSurfaceCapabilitiesKHR capabilities;
  // • Surface formats (pixel format, color space)
  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  // • Available presentation modes
  std::vector<VkPresentModeKHR> presentModes;
};

class SwapChain
{
  private:
  VkRenderPass renderPass;
  VkFormat swapChainImageFormat;
  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;

  // Depth image and view configuration.
  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;


  /**
   * We'll need one semaphore to signal that an image has been acquired from the swapchain and is ready for rendering,
   * another one to signal that rendering has finished and presentation can happen, and a fence to make sure only one frame is rendering at a time.
   */
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

  // Cache
  VkDevice cachedDevice;

  // Depth Configuration
  VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
  VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

public:
  SwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface);
  ~SwapChain();

  size_t currentFrame = 0;

  const int MAX_FRAMES_IN_FLIGHT = 2;

  /**
   * std::vector<VkImage> depthImages;
   * std::vector<VkDeviceMemory> depthImageMemorys;
   * std::vector<VkImageView> depthImageViews;
   * VkExtent2D windowExtent;
   */ 

  
  void createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface);
  void recreateSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, 
                         VkQueue graphicsQueue, VkCommandPool commandPool, VkSurfaceKHR surface);
  void createImageViews(VkDevice device);
  void createDepthResources(VkDevice device, VkPhysicalDevice physicalDevice, 
                            VkQueue graphicsQueue, VkCommandPool commandPool);
  void createRenderPass(VkDevice device, VkPhysicalDevice physicalDevice);
  void createFramebuffers(VkDevice device);
  void createSyncObjects(VkDevice device);
  void restartSwapChain(VkDevice device);

  // Helper functions

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  // Getters and setters

  VkFormat getSwapChainImageFormat();
  VkRenderPass getRenderPass();
  VkSwapchainKHR getSwapChain();
  VkExtent2D getSwapChainExtent();
  std::vector<VkFramebuffer> getSwapChainFramebuffers();
  std::vector<VkSemaphore> getImageAvailableSemaphores();
  std::vector<VkSemaphore> getRenderFinishedSemaphores();
  std::vector<VkFence> getInFlightFences();
  std::vector<VkFence> getImagesInFlight();
};