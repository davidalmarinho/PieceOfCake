#include <limits>

#include "SwapChain.hpp"
#include "QueueFamilyIndices.hpp"
#include "Pipeline.hpp"
#include "Engine.hpp"
#include "Utils.hpp"

SwapChain::SwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkSampleCountFlagBits numMsaaSamples) : cachedDevice(device), cachedMsaaSample(numMsaaSamples)
{
  this->createSwapChain(physicalDevice, device, surface);
  this->createImageViews(device);
  this->createRenderPass(device, physicalDevice, numMsaaSamples);
}

void SwapChain::createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface)
{
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.surfaceFormats);
  VkPresentModeKHR presentMode     = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent                = chooseSwapExtent(swapChainSupport.capabilities);

  // Specify how many images we want in the swap chain
  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  // +1 because it is recommended to ask for at least 1 more image
  // than the minimum.

  // Don't exceed the maximum number of images
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  // Finally, actually creating and integrating swap chain into Vulkan.
  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;

  // Specifying swap chain details.
  createInfo.minImageCount    = imageCount;
  createInfo.imageFormat      = surfaceFormat.format;
  createInfo.imageColorSpace  = surfaceFormat.colorSpace;
  createInfo.imageExtent      = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // Here we’re going to render directly to the images, which
  // means that they’re used as color attachment. It is also possible
  // render images to a separate image first to perform operations
  // like post-processing.
  // In that case we may use a value like
  // VK_IMAGE_USAGE_TRANSFER_DST_BIT instead
  // and use a memory operation to transfer the rendered image
  // to a swap chain image.

  QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices   = queueFamilyIndices;
  }
  else {
    createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices   = nullptr;
  }

  // • VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family
  // at a time and ownership must be explicitly transferred before using it in
  // another queue family. This option offers the best performance.
  // It is even nicer when presentation family and graphics family
  // are the same
  // • VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue
  // families without explicit ownership transfers.

  createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode    = presentMode;
  createInfo.clipped        = VK_TRUE; // We don’t care about the color of pixels that are obscured

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create swap chain!\n");
  }

  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

SwapChain::~SwapChain()
{
  this->clean(cachedDevice, cachedMsaaSample);
}

void SwapChain::clean(VkDevice device, VkSampleCountFlagBits msaaSample)
{
  this->restartSwapChain(device, msaaSample);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(device, inFlightFences[i], nullptr);
  }

  vkDestroyRenderPass(device, renderPass, nullptr);
}

void SwapChain::restartSwapChain(VkDevice device, VkSampleCountFlagBits msaaSample)
{
  // Destroys color images if it has been created due to MSAA activation.
  if (msaaSample != VK_SAMPLE_COUNT_1_BIT) {
    vkDestroyImageView(device, colorImageView, nullptr);
    vkDestroyImage(device, colorImage, nullptr);
    vkFreeMemory(device, colorImageMemory, nullptr);
  }

  vkDestroyImageView(device, depthImageView, nullptr);
  vkDestroyImage(device, depthImage, nullptr);
  vkFreeMemory(device, depthImageMemory, nullptr);

  for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
    vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
  }

  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    vkDestroyImageView(device, swapChainImageViews[i], nullptr);
  }

  vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void SwapChain::recreateSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, 
                                  VkQueue graphicsQueue, VkCommandPool commandPool, VkSurfaceKHR surface,
                                  VkSampleCountFlagBits msaaSamples)
{
  this->restartSwapChain(device, msaaSamples);

  this->createSwapChain(physicalDevice, device, surface);
  this->createImageViews(device);
  this->createColorResources(device, physicalDevice, msaaSamples);
  this->createDepthResources(device, physicalDevice, graphicsQueue, commandPool, msaaSamples);
  this->createFramebuffers(device, msaaSamples);
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  else {
    int width, height;
    glfwGetFramebufferSize(Engine::get()->getWindow()->getGlfwWindow(), &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width  = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

void SwapChain::createImageViews(VkDevice device)
{
  // Resize the list to fit all of the image views we’ll be creating
  swapChainImageViews.resize(swapChainImages.size());

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    swapChainImageViews[i] = Utils::createImageView(device, swapChainImages[i], 
                                                    swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
}

void SwapChain::createRenderPass(VkDevice device, VkPhysicalDevice physicalDevice, VkSampleCountFlagBits msaaSamples)
{
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format  = swapChainImageFormat;
  colorAttachment.samples = msaaSamples;

  colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Specifies which layout the image will have before the render pass begins.

  // Specify the layout to automatically transition to when the render pass finishes.
  if (msaaSamples ==  VK_SAMPLE_COUNT_1_BIT)
    colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  else
    colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format         = findDepthFormat(physicalDevice);
  depthAttachment.samples        = msaaSamples;
  depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachmentResolve{};
  colorAttachmentResolve.format = swapChainImageFormat;
  colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0; // Specifies which attachment to reference by its index in the attachment descriptions array.
  colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachmentResolveRef{};
  colorAttachmentResolveRef.attachment = 2;
  colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount    = 1; // The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive.
  subpass.pColorAttachments       = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;
  if (msaaSamples != VK_SAMPLE_COUNT_1_BIT) {
    subpass.pResolveAttachments = &colorAttachmentResolveRef;
  } 

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;

  dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;

  dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::vector<VkAttachmentDescription> attachments;
  if (msaaSamples == VK_SAMPLE_COUNT_1_BIT)
    attachments = {colorAttachment, depthAttachment};
  else
    attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments    = attachments.data();
  renderPassInfo.subpassCount    = 1;
  renderPassInfo.pSubpasses      = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies   = &dependency;

  if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create the render pass.\n");
  }
}

void SwapChain::createFramebuffers(VkDevice device, VkSampleCountFlagBits msaaSample)
{
  // Resize the container to hold all of the framebuffers
  swapChainFramebuffers.resize(swapChainImageViews.size());

  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    std::vector<VkImageView> attachments;

    if (msaaSample == VK_SAMPLE_COUNT_1_BIT)
      attachments = {swapChainImageViews[i], depthImageView};
    else
      attachments = {colorImageView, depthImageView, swapChainImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass      = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments    = attachments.data();
    framebufferInfo.width           = swapChainExtent.width;
    framebufferInfo.height          = swapChainExtent.height;
    framebufferInfo.layers          = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Error: Failed to create framebuffer.\n");
    }
  }
}

void SwapChain::createSyncObjects(VkDevice device)
{
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  // Config Semaphore.
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  // Config Fence.
  VkFenceCreateInfo fenceInfo{};
  // fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    // Create Semaphores and Fences.
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create synchronization objects for a frame.\n");
    }
  }
}

SwapChainSupportDetails SwapChain::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.surfaceFormats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

// Setting up Surface Format --depth color
VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

// Setting up Presentation Mode --conditions for swaping images to the screen.
VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR && Engine::get()->getWindow()->isVsyncEnabled())
      return availablePresentMode;
    else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR && !Engine::get()->getWindow()->isVsyncEnabled())
      return availablePresentMode;
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

// MSAA configuration.
void SwapChain::createColorResources(VkDevice device, VkPhysicalDevice physicalDevice, 
                                     VkSampleCountFlagBits msaaSamples)
{
  if (Engine::get()->getRenderer()->msaaSetting != Renderer::MsaaSetting::DISABLED) {
    VkFormat colorFormat = swapChainImageFormat;

    Utils::createImage(device, physicalDevice, swapChainExtent.width, swapChainExtent.height, 
                      1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, 
                      VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
    colorImageView = Utils::createImageView(device, colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
}

// Depth configuration.

void SwapChain::createDepthResources(VkDevice device, VkPhysicalDevice physicalDevice, 
                                     VkQueue graphicsQueue, VkCommandPool commandPool, VkSampleCountFlagBits numMsaaSamples)
{
  VkFormat depthFormat = findDepthFormat(physicalDevice);
  
  Utils::createImage(device, physicalDevice, swapChainExtent.width, swapChainExtent.height, 1, numMsaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
  this->depthImageView = Utils::createImageView(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

  Utils::transitionImageLayout(device, graphicsQueue, commandPool, 
                               depthImage, depthFormat, 
                               VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

// Takes a list of candidate formats in order from most desirable to least desirable, and checks which is the first one that is supported.
VkFormat SwapChain::findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, 
                                        VkImageTiling tiling, VkFormatFeatureFlags features)
{
  // The support of a format depends on the tiling mode and usage, so we must also include these as parameters.
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } 
    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("Error: Failed to find supported format.\n");
}

// Helper function to select a format with a depth component that supports usage as depth attachment.
VkFormat SwapChain::findDepthFormat(VkPhysicalDevice physicalDevice)
{
  return findSupportedFormat(physicalDevice,
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

// Getters and Setters

VkFormat SwapChain::getSwapChainImageFormat()
{
  return this->swapChainImageFormat;
}

VkRenderPass SwapChain::getRenderPass()
{
  return this->renderPass;
}

VkSwapchainKHR SwapChain::getSwapChain()
{
  return this->swapChain;
}

VkExtent2D SwapChain::getSwapChainExtent()
{
  return this->swapChainExtent;
}

std::vector<VkFramebuffer> SwapChain::getSwapChainFramebuffers()
{
  return this->swapChainFramebuffers;
}

std::vector<VkSemaphore> SwapChain::getImageAvailableSemaphores()
{
  return this->imageAvailableSemaphores;
}

std::vector<VkSemaphore> SwapChain::getRenderFinishedSemaphores()
{
  return this->renderFinishedSemaphores;
}

std::vector<VkFence> SwapChain::getInFlightFences()
{
  return this->inFlightFences;
}

std::vector<VkFence> SwapChain::getImagesInFlight()
{
  return this->imagesInFlight;
}

int SwapChain::getSwapChainImageViewsSize()
{
  return this->swapChainImageViews.size();
}
