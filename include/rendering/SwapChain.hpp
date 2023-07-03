#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>

#include "Window.hpp"

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
public:
  SwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, Window* window);
  ~SwapChain();

  const int MAX_FRAMES_IN_FLIGHT = 2;

  /**
   * std::vector<VkImage> depthImages;
   * std::vector<VkDeviceMemory> depthImageMemorys;
   * std::vector<VkImageView> depthImageViews;
   * VkExtent2D windowExtent;
   */ 

  /**
   * We'll need one semaphore to signal that an image has been acquired from the swapchain and is ready for rendering,
   * another one to signal that rendering has finished and presentation can happen, and a fence to make sure only one frame is rendering at a time.
   */
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
  
  void createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, Window* window);
  void recreateSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, Window* window);
  void createImageViews(VkDevice device);
  // void createDepthResources();
  void createRenderPass(VkDevice device);
  void createFramebuffers(VkDevice device);
  void createSyncObjects(VkDevice device);
  void restartSwapChain(VkDevice device);

  // Helper functions
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, Window* window);

  // Getters and setters
  VkFormat getSwapChainImageFormat();
  VkRenderPass getRenderPass();

  // TODO: There are public variables that mustn't be public.
  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkRenderPass renderPass;

private:
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes, Window* window);


  // Cache
  VkDevice cachedDevice;
};