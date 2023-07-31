#include <unordered_map>
#include <algorithm>
#include <optional>
#include <iostream>

#include "Renderer.hpp"
#include "Engine.hpp"

#include "ModelRenderer.hpp"
#include "Utils.hpp"

#ifdef IMGUI_ENABLED
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "ImGuiLayer.hpp"
#endif

Renderer::MipmapSetting operator++(Renderer::MipmapSetting& mipmapSetting, int)
{
  switch(mipmapSetting) {
    case Renderer::MipmapSetting::DISABLED:
      mipmapSetting = Renderer::MipmapSetting::LINEAR;
      break;
    case Renderer::MipmapSetting::LINEAR:
      mipmapSetting = Renderer::MipmapSetting::DISABLED;
      break;
  }

  return mipmapSetting;
}

std::ostream& operator<<(std::ostream& os, Renderer::MipmapSetting mipmapSetting)
{
  switch(mipmapSetting) {
    case Renderer::MipmapSetting::DISABLED: 
      os << "Disabled"; 
      return os;
    case Renderer::MipmapSetting::LINEAR: 
      os << "Linear"; 
      return os;
    case Renderer::MipmapSetting::NEAREST: 
      os << "Nearest"; 
      return os;
    default:
      os << "???"; 
      return os;
  }
}

Renderer::MsaaSetting operator++(Renderer::MsaaSetting& msaaSetting, int)
{
  switch(msaaSetting) {
    case Renderer::MsaaSetting::DISABLED:
      if (static_cast<int>(Engine::get()->getRenderer()->maxMsaaSamples) == static_cast<int>(Renderer::MsaaSetting::DISABLED)) {
        msaaSetting = Renderer::MsaaSetting::DISABLED;
        break;
      }
      msaaSetting = Renderer::MsaaSetting::MSAA2X;
      break;
    case Renderer::MsaaSetting::MSAA2X:
      if (static_cast<int>(Engine::get()->getRenderer()->maxMsaaSamples) == static_cast<int>(Renderer::MsaaSetting::MSAA2X)) {
        msaaSetting = Renderer::MsaaSetting::DISABLED;
        break;
      }
      msaaSetting = Renderer::MsaaSetting::MSAA4X;
      break;
    case Renderer::MsaaSetting::MSAA4X:
      if (static_cast<int>(Engine::get()->getRenderer()->maxMsaaSamples) == static_cast<int>(Renderer::MsaaSetting::MSAA4X)) {
        msaaSetting = Renderer::MsaaSetting::DISABLED;
        break;
      }
      msaaSetting = Renderer::MsaaSetting::MSAA8X;
      break;
    case Renderer::MsaaSetting::MSAA8X:
      if (static_cast<int>(Engine::get()->getRenderer()->maxMsaaSamples) == static_cast<int>(Renderer::MsaaSetting::MSAA8X)) {
        msaaSetting = Renderer::MsaaSetting::DISABLED;
        break;
      }
      msaaSetting = Renderer::MsaaSetting::MSAA16X;
      break;
    case Renderer::MsaaSetting::MSAA16X:
      if (static_cast<int>(Engine::get()->getRenderer()->maxMsaaSamples) == static_cast<int>(Renderer::MsaaSetting::MSAA16X)) {
        msaaSetting = Renderer::MsaaSetting::DISABLED;
        break;
      }
      msaaSetting = Renderer::MsaaSetting::MSAA32X;
      break;
    case Renderer::MsaaSetting::MSAA32X:
      if (static_cast<int>(Engine::get()->getRenderer()->maxMsaaSamples) == static_cast<int>(Renderer::MsaaSetting::MSAA32X)) {
        msaaSetting = Renderer::MsaaSetting::DISABLED;
        break;
      }
      msaaSetting = Renderer::MsaaSetting::MSAA64X;
      break;
    case Renderer::MsaaSetting::MSAA64X:
      msaaSetting = Renderer::MsaaSetting::DISABLED;
      break;
  }

  return msaaSetting;
}

std::ostream& operator<<(std::ostream& os, Renderer::MsaaSetting msaaSetting)
{
  switch(msaaSetting) {
    case Renderer::MsaaSetting::DISABLED: 
      os << "Disabled"; 
      return os;
    case Renderer::MsaaSetting::MSAA2X: 
      os << "MSAA2X";
      return os;
    case Renderer::MsaaSetting::MSAA4X:
      os << "MSAA4X";
      return os;
    case Renderer::MsaaSetting::MSAA8X:
      os << "MSAA8X";
      return os;
    case Renderer::MsaaSetting::MSAA16X:
      os << "MSAA16X";
      return os;
    case Renderer::MsaaSetting::MSAA32X:
      os << "MSAA32X";
      return os;
    case Renderer::MsaaSetting::MSAA64X:
      os << "MSAA64X";
      return os;
    default:
      os << "???"; 
      return os;
  }
}

/**
 * @brief Calculates the number of samples that the hardware can handle.
 * 
 * @return VkSampleCountFlagBits 
 */
VkSampleCountFlagBits Renderer::getMaxUsableSampleCount()
{
  VkPhysicalDeviceProperties physicalDeviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

  VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & 
                              physicalDeviceProperties.limits.framebufferDepthSampleCounts;
  if (counts & VK_SAMPLE_COUNT_64_BIT) {
    std::cout << "INFO: Using VK_SAMPLE_COUNT_64_BIT\n";
    return VK_SAMPLE_COUNT_64_BIT; 
  }
  if (counts & VK_SAMPLE_COUNT_32_BIT) {
    std::cout << "INFO: Using VK_SAMPLE_COUNT_32_BIT\n";
    return VK_SAMPLE_COUNT_32_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_16_BIT) {
    std::cout << "INFO: Using VK_SAMPLE_COUNT_16_BIT\n";
    return VK_SAMPLE_COUNT_16_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_8_BIT) {
    std::cout << "INFO: Using VK_SAMPLE_COUNT_8_BIT\n";
    return VK_SAMPLE_COUNT_8_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_4_BIT) {
    std::cout << "INFO: Using VK_SAMPLE_COUNT_4_BIT\n";
    return VK_SAMPLE_COUNT_4_BIT; 
  }
  if (counts & VK_SAMPLE_COUNT_2_BIT) {
    std::cout << "INFO: Using VK_SAMPLE_COUNT_2_BIT\n";
    return VK_SAMPLE_COUNT_2_BIT;
  }

  std::cout << "INFO: Using VK_SAMPLE_COUNT_1_BIT\n";
  return VK_SAMPLE_COUNT_1_BIT;
}

Renderer::Renderer()
{
  
}

void Renderer::init()
{
  // Show Vulkan version 
  auto FN_vkEnumerateInstanceVersion = PFN_vkEnumerateInstanceVersion(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

  uint32_t instanceVersion;
  if(FN_vkEnumerateInstanceVersion == nullptr)
    instanceVersion = VK_API_VERSION_1_0;
  else {
    auto result = FN_vkEnumerateInstanceVersion(&instanceVersion);
  }

  uint32_t major = VK_VERSION_MAJOR(instanceVersion);
  uint32_t minor = VK_VERSION_MINOR(instanceVersion);
  uint32_t patch = VK_VERSION_PATCH(instanceVersion);

  std::cout << "Hello Vulkan Version " << major << "." << minor << "." << patch << std::endl;
    
  initVulkan();
}

void Renderer::initVulkan()
{
  // TODO: There are 3 types of mipmapping: Nearest, Linear and disabled. Nearest has to be implemented.
  createInstance();
  this->vulkanDebugger = std::make_unique<VulkanDebugger>(this->vkInstance);
  Engine::get()->getWindow()->createSurface(this->vkInstance, &this->surface);
  pickPhysicalDevice();

  // this->msaaSamples = VK_SAMPLE_COUNT_1_BIT;

  createLogicalDevice();
}

void Renderer::initRendering()
{
  this->swapChain = std::make_unique<SwapChain>(physicalDevice, device, surface, msaaSamples);
  for (int i = 0; i < this->entitiesVec.size(); i++) {
    std::unique_ptr<Pipeline> pipe = std::make_unique<Pipeline>(device, swapChain->getRenderPass());
    pipe->createGraphicsPipeline(device, swapChain->getSwapChainImageFormat(), swapChain->getRenderPass(), this->msaaSamples);
    this->pipelines.push_back(std::move(pipe));
  }

  createCommandPool(&commandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  this->swapChain->createColorResources(device, physicalDevice, msaaSamples);
  this->swapChain->createDepthResources(device, physicalDevice, graphicsQueue, commandPool, msaaSamples);
  this->swapChain->createFramebuffers(device, msaaSamples);

  std::shared_ptr<Texture> tex = AssetPool::getTexture("img_tex");
  tex->createTextureImage(device, physicalDevice, graphicsQueue, commandPool);
  tex->createTextureImageView(device);
  tex->createTextureSampler(device, physicalDevice);

  AssetPool::loadModels();

  std::shared_ptr<Shader> shader = AssetPool::getShader("texture");
  for (int i = 0; i < this->pipelines.size(); i++) {
    this->pipelines[i]->createUniformBuffers();
    this->pipelines[i]->getDescriptorLayout()->createDescriptorPool();
    this->pipelines[i]->getDescriptorLayout()->createDescriptorSets(pipelines[i].get(), tex.get());
  }

  createCommandBuffers();
  this->swapChain->createSyncObjects(device);

#ifdef IMGUI_ENABLED
  this->initGui();
#endif
}

void Renderer::restart()
{
  // Destruction
  vkDeviceWaitIdle(device);

#ifdef IMGUI_ENABLED
  this->cleanGui();
#endif

  // Convert MsaaSetting enum into the VkSampleCountFlagBits enum.
  msaaSamples = static_cast<VkSampleCountFlagBits>(static_cast<int>(msaaSetting));

  this->swapChain.reset();

  std::shared_ptr<Texture> tex1 = AssetPool::getTexture("img_tex");
  tex1->clean(device);

  for (int i = 0; i < this->pipelines.size(); i++) {
    this->pipelines[i].reset();
  }
  this->pipelines.clear();

  // Recreation
  this->swapChain = std::make_unique<SwapChain>(physicalDevice, device, surface, msaaSamples);
  for (int i = 0; i < this->entitiesVec.size(); i++) {
    std::unique_ptr<Pipeline> pipe = std::make_unique<Pipeline>(device, swapChain->getRenderPass());
    pipe->createGraphicsPipeline(device, swapChain->getSwapChainImageFormat(), swapChain->getRenderPass(), this->msaaSamples);
    this->pipelines.push_back(std::move(pipe));
  }

  this->swapChain->createColorResources(device, physicalDevice, msaaSamples);
  this->swapChain->createDepthResources(device, physicalDevice, graphicsQueue, commandPool, msaaSamples);
  this->swapChain->createFramebuffers(device, msaaSamples);

  std::shared_ptr<Texture> tex = AssetPool::getTexture("img_tex");
  tex->createTextureImage(device, physicalDevice, graphicsQueue, commandPool);
  tex->createTextureImageView(device);
  tex->createTextureSampler(device, physicalDevice);

  std::shared_ptr<Shader> shader = AssetPool::getShader("texture");

  for (int i = 0; i < this->pipelines.size(); i++) {
    this->pipelines[i]->createUniformBuffers();
    this->pipelines[i]->getDescriptorLayout()->createDescriptorPool();
    this->pipelines[i]->getDescriptorLayout()->createDescriptorSets(pipelines[i].get(), tex.get());
  }

  this->swapChain->createSyncObjects(device);

#ifdef IMGUI_ENABLED
  this->initGui();
#endif
}

Renderer::~Renderer()
{
  this->clean();
}

void Renderer::clean()
{
  vkDeviceWaitIdle(device);

#ifdef IMGUI_ENABLED
  this->cleanGui();
#endif

  AssetPool::cleanup();
  for (int i = 0; i < this->pipelines.size(); i++) {
    this->pipelines[i].reset();
  }
  this->swapChain.reset();

  vkDestroyCommandPool(device, commandPool, nullptr);

  vkDestroyDevice(device, nullptr);

  if (VulkanDebugger::ENABLE_VALIDATION_LAYERS) {
    this->vulkanDebugger->destroyDebugUtilsMessengerEXT(this->vkInstance, nullptr);
  }

  vkDestroySurfaceKHR(this->vkInstance, surface, nullptr);
  vkDestroyInstance(this->vkInstance, nullptr);
}

void Renderer::createInstance()
{
  if (VulkanDebugger::ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
    throw std::runtime_error("Validation layers requested, but not available.\n");
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
  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (VulkanDebugger::ENABLE_VALIDATION_LAYERS) {
    createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    this->vulkanDebugger->populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT *>(&debugCreateInfo);
  }
  else {
    createInfo.enabledLayerCount = 0;

    createInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &this->vkInstance) != VK_SUCCESS) {
    throw std::runtime_error("Error: Couldn't create VkInstance\n");
  }
}

void Renderer::pickPhysicalDevice()
{
  // List graphics card
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, nullptr);

  // Throw error if we haven't find graphics card
  if (deviceCount == 0) {
    throw std::runtime_error("Error: Failed to find GPUs with Vulkan support.\n");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, devices.data());

  // Check if the computer has a graphics card that Vulkan can handle all
  // the operations that it needs.
  for (const auto &device : devices) {
    if (isDeviceSuitable(device))
    {
      physicalDevice = device;
      maxMsaaSamples = getMaxUsableSampleCount();
      msaaSamples  = maxMsaaSamples;
      msaaSetting = static_cast<Renderer::MsaaSetting>(static_cast<int>(msaaSamples));
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("Error: Failed to find a suitable GPU.\n");
  }
}

void Renderer::createLogicalDevice()
{
  // Specifies the number of queues we want for a single queue family.
  // We gonna specify it just to be a queue which supports graphics capabilities.
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  // Creating queues
  // TODO: In future is a good idea to make this multithread.
  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE; // Enable Anisotropy device feature.
  
  if (sampleShading) {
    deviceFeatures.sampleRateShading = VK_TRUE; // Enable sample shading feature for the device.
  }

  // Creating Logical Device.
  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  // Pointers to the queue creation info and device features structs
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;

  // Turn on swap chain system.
  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  // Guarantee compatibility with older devices and older vulkan devices.
  // Because this isn't needed anymore.
  if (VulkanDebugger::ENABLE_VALIDATION_LAYERS) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }
  else {
    createInfo.enabledLayerCount = 0;
  }

  // NOTE: Here we can specify extensions to do more cool stuff with vulkan.

  // Instantiate the logical.
  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
    throw std::runtime_error("Error: Logical Device creation has failed!\n");
  }

  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void Renderer::recreateSwapChain()
{
  int width = 0, height = 0;
  glfwGetFramebufferSize(Engine::get()->getWindow()->getGlfwWindow(), &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(Engine::get()->getWindow()->getGlfwWindow(), &width, &height);
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(device);

  this->swapChain->recreateSwapChain(device, physicalDevice, graphicsQueue, 
                                     commandPool, surface, msaaSamples);
}

void Renderer::createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags commandPoolCreateFlags)
{
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = commandPoolCreateFlags;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  if (vkCreateCommandPool(device, &poolInfo, nullptr, commandPool) != VK_SUCCESS) {
    throw std::runtime_error("Error: Command Pool creation has failed.\n");
  }
}

void Renderer::createCommandBuffers()
{
  commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  // Allocate command buffers
  if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to allocate Command buffers.\n");
  }
}

void Renderer::createCommandBuffer(VkCommandBuffer* commandBuffer, uint32_t commandBufferCount, VkCommandPool &commandPool)
{
  VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
  commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandPool = commandPool;
  commandBufferAllocateInfo.commandBufferCount = commandBufferCount;
  if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to allocate a command buffer.\n");
  }
}

/**
   * @brief Function that writes the commands we want to execute into a command buffer.
   *
   * @param commandBuffer
   * @param imageIndex
   */
void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to begin recording command buffer.");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass  = this->swapChain->getRenderPass();
  renderPassInfo.framebuffer = swapChain->getSwapChainFramebuffers()[imageIndex];

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width  = static_cast<float>(swapChain->getSwapChainExtent().width);
  viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapChain->getSwapChainExtent();
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  // Bind Graphics Pipeline
  for (int i = 0; i < this->entitiesVec.size(); i++) {
    this->pipelines[i]->bind(commandBuffer);

    std::shared_ptr<Model> model = this->entitiesVec[i].get().getComponent<ModelRenderer>().model.lock();
    model->bind(commandBuffer);

    this->pipelines[i]->getDescriptorLayout()->bind(pipelines[i].get(), commandBuffer);
    model->draw(commandBuffer);
  }

#ifdef IMGUI_ENABLED
  this->renderImGui(commandBuffer);
#endif

  vkCmdEndRenderPass(commandBuffer);

  // Finished recording the command buffer.
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to record Command Buffer.\n");
  }
}

void Renderer::drawFrame()
{
  // Wait until the previous frame has finished.
  vkWaitForFences(device, 1, &(swapChain->getInFlightFences()[swapChain->currentFrame]), VK_TRUE, UINT64_MAX);

  // Acquire an image from the swap chain.
  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(device, swapChain->getSwapChain(), 
                    UINT64_MAX, swapChain->getImageAvailableSemaphores()[swapChain->currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) { // Means that the window has been rezised and now we have to recreate the swapchain
    recreateSwapChain();
    return;
  } 
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("Error: Failed to acquire swap chain image.");
  }

  // Update uniform buffers.
  for (int i = 0; i < this->pipelines.size(); i++) {
    this->pipelines[i]->updateUniformBuffer(this->swapChain->currentFrame, i);
  }

  // Only reset the fence if we are submitting work.
  vkResetFences(device, 1, &(swapChain->getInFlightFences()[swapChain->currentFrame]));

  vkResetCommandBuffer(commandBuffers[swapChain->currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
  recordCommandBuffer(commandBuffers[swapChain->currentFrame], imageIndex);

  // Queue submission and synchronization.
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {swapChain->getImageAvailableSemaphores()[swapChain->currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[swapChain->currentFrame];

  VkSemaphore signalSemaphores[] = {swapChain->getRenderFinishedSemaphores()[swapChain->currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, swapChain->getInFlightFences()[swapChain->currentFrame]) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to submit draw command buffer.");
  }

  // Submit the result back to the swap chain.
  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain->getSwapChain()};
  presentInfo.swapchainCount  = 1;
  presentInfo.pSwapchains     = swapChains;

  presentInfo.pImageIndices = &imageIndex;

  // Submit the request to present an image to the swap chain.
  result = vkQueuePresentKHR(presentQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Engine::get()->getWindow()->framebufferResized) {
    Engine::get()->getWindow()->framebufferResized = false;
    recreateSwapChain();
  } 
  else if (result != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to present swap chain image.\n");
  }

  // Go to next frame
  swapChain->currentFrame = (swapChain->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

/**
  * Check if a graphics card can handle with all Vulkan operations
  *
  * @return True if the graphics card can handle with all operations that vulkan
  *		   needs, otherwise returns False.
  */
bool Renderer::isDeviceSuitable(VkPhysicalDevice device)
{
  // Get device properties so we can check if the graphics cards
  // support geometry shaders, in this case
  /*
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(this->m_physicalDevice, &deviceProperties);
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(this->m_physicalDevice, &deviceFeatures);

  return deviceProperties.deviceType ==
    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
  */

  QueueFamilyIndices indices = findQueueFamilies(device, surface);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = swapChain->querySwapChainSupport(device, surface);
    swapChainAdequate = !swapChainSupport.surfaceFormats.empty() && !swapChainSupport.presentModes.empty();
  }

  // Check if the device supports Anisotropy device feature.
  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

/**
  * @brief Checks if the device has a graphics cards which can handle with
  *        the swap chain mechaninsm. So, we need a graphics card capable
  *        of drawing images and act as FrameBuffer
  *        --When an image is been draw, we 1 or more images prepared to
  *          be called for rendering.
  *
  * @param vkDevice Vulkan physical device.
  * @return true if there is a graphics card capable of handling swap chain mechanism.
  */
bool Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

/**
  * @brief Get extensions which are required by vulkan, like:
  *		  -> VK_KHR_surface
  *
  * @return
  */
std::vector<const char *> Renderer::getRequiredExtensions()
{
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (VulkanDebugger::ENABLE_VALIDATION_LAYERS) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

bool Renderer::checkValidationLayerSupport()
{
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  // Check if the required layers are available
  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
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

void Renderer::addEntity(Entity& e)
{
  // TODO: modelVec.empty() || !(std::count(modelVec.begin(), modelVec.end(), model));
  bool found = false;
  ModelRenderer& m = e.getComponent<ModelRenderer>();
  if (!found) {
    entitiesVec.insert(entitiesVec.begin(), e);
  }
}

#ifdef IMGUI_ENABLED
void Renderer::initGui()
{
  // Create descriptor pool for imgui. Copied from imgui demo itself.
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	vkCreateDescriptorPool(this->device, &pool_info, nullptr, &imguiPool);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();
  // ImGui::StyleColorsClassic();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForVulkan(Engine::get()->getWindow()->getGlfwWindow(), true);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = this->vkInstance;
  init_info.PhysicalDevice = this->physicalDevice;
  init_info.Device = this->device;
  init_info.Queue = this->graphicsQueue;
  init_info.PipelineCache = VK_NULL_HANDLE;
  init_info.DescriptorPool = imguiPool;
  init_info.Allocator = VK_NULL_HANDLE;
  init_info.MinImageCount = 2;
  init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
  init_info.CheckVkResultFn = 0;
  init_info.MSAASamples = this->msaaSamples;

  ImGui_ImplVulkan_Init(&init_info, this->swapChain->getRenderPass());

  VkCommandBuffer commandBuffer = Utils::beginSingleTimeCommands(this->device, this->commandPool);
  ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
  Utils::endSingleTimeCommands(this->device, this->graphicsQueue, this->commandPool, commandBuffer);

  vkDeviceWaitIdle(this->device);
  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Renderer::renderImGui(VkCommandBuffer commandBuffer)
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowDemoWindow();
  ImGuiLayer::render();
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void Renderer::cleanGui()
{
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  vkDestroyDescriptorPool(device, imguiPool, nullptr);
}
#endif

// Getters and Setters

VkDevice Renderer::getDevice()
{
  return this->device;
}

VkPhysicalDevice Renderer::getPhysicalDevice()
{
  return this->physicalDevice;
}

VkCommandPool Renderer::getCommandPool()
{
  return this->commandPool;
}

VkQueue Renderer::getGraphicsQueue()
{
  return this->graphicsQueue;
}

const std::unique_ptr<SwapChain> &Renderer::getSwapChain() const
{
  return this->swapChain;
}

VkSampleCountFlagBits Renderer::getMsaaSample()
{
  return this->msaaSamples;
}

VkSampleCountFlagBits Renderer::getMaxMsaaSamples()
{
  return this->maxMsaaSamples;
}

const std::optional<std::reference_wrapper<Entity>> Renderer::getEntity(int index)
{
  if (index >= entitiesVec.size()) {
    std::cout << "Warning: No rendering entity in '" << index << "' index.\n";
    return {};
  }

  return this->entitiesVec[index];
}