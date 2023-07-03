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

class HelloTriangleApplication
{
public:
  void run()
  {
    this->window = std::make_unique<Window>("Vulkan", 800, 600, false);
    this->window->init();
    this->window->showVersion();

    // Show Vulkan version 
    uint32_t instanceVersion = VK_API_VERSION_1_0;
    auto FN_vkEnumerateInstanceVersion = PFN_vkEnumerateInstanceVersion(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
    if(vkEnumerateInstanceVersion){
      vkEnumerateInstanceVersion(&instanceVersion );
    }

    uint32_t major = VK_VERSION_MAJOR(instanceVersion);
    uint32_t minor = VK_VERSION_MINOR(instanceVersion);
    uint32_t patch = VK_VERSION_PATCH(instanceVersion);

    std::cout << "Hello Vulkan Version:" << major << "." << minor << "." << patch << std::endl;
    
    initVulkan();
    mainLoop();
    cleanup();
  }

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

  void initVulkan()
  {
    createInstance();
    this->vulkanDebugger = std::make_unique<VulkanDebugger>(this->vkInstance);
    this->window->createSurface(this->vkInstance, &this->surface);
    pickPhysicalDevice();
    createLogicalDevice();
    this->swapChain = std::make_unique<SwapChain>(physicalDevice, device, surface, window.get());
    this->pipeline = std::make_unique<Pipeline>(device, swapChain->renderPass);
    this->pipeline->createGraphicsPipeline(device, swapChain->getSwapChainImageFormat(), swapChain->getRenderPass());
    this->swapChain->createFramebuffers(device);
    createCommandPool();
    this->pipeline->createVertexBuffer(device, physicalDevice, commandPool, graphicsQueue);
    this->pipeline->createIndexBuffer(device, physicalDevice, 
                                      commandPool, graphicsQueue);
    createCommandBuffers();
    this->swapChain->createSyncObjects(device);
  }

  void mainLoop()
  {
    float lastTime = glfwGetTime();
    float accumulator = 0.0f;
    const unsigned short MAX_FPS = 3000;
    const float MAX_FPS_PER_SEC = 1.0f / MAX_FPS;

    // Show FPS / Second
    float timer = 0.0f;
    unsigned short fps = 0;
    const uint8_t ONE_SECOND = 1;

    while (!this->window->isReadyToClose()) {
      float currentTime = glfwGetTime();
      float delta = currentTime - lastTime;

      timer += delta;
      accumulator += delta;

      lastTime = currentTime;

      // Controls frame rate.
      if (accumulator > MAX_FPS_PER_SEC) {
        KeyListener::update();
        glfwPollEvents();

        // Call engine logic
        accumulator = 0.0f;
        fps++;
        drawFrame();
      }

      // Get fps per second.
      if (timer > ONE_SECOND) {
        std::cout << "FPS: " << fps << "\n";
        fps = 0;
        timer = 0.0f;
      }

    }

    vkDeviceWaitIdle(device);
  }

  void cleanup()
  {
    this->swapChain.reset();

    this->pipeline.reset();

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (VulkanDebugger::ENABLE_VALIDATION_LAYERS)
    {
      this->vulkanDebugger->destroyDebugUtilsMessengerEXT(this->vkInstance, nullptr);
    }

    vkDestroySurfaceKHR(this->vkInstance, surface, nullptr);
    vkDestroyInstance(this->vkInstance, nullptr);

    this->window.reset();
    
  }

  void createInstance()
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
    if (VulkanDebugger::ENABLE_VALIDATION_LAYERS)
    {
      createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      this->vulkanDebugger->populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT *>(&debugCreateInfo);
    }
    else
    {
      createInfo.enabledLayerCount = 0;

      createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &this->vkInstance) != VK_SUCCESS)
    {
      throw std::runtime_error("Error: Couldn't create VkInstance\n");
    }
  }

  void pickPhysicalDevice()
  {
    // List graphics card
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, nullptr);

    // Throw error if we haven't find graphics card
    if (deviceCount == 0)
    {
      throw std::runtime_error("Error: Failed to find GPUs with Vulkan support.\n");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, devices.data());

    // Check if the computer has a graphics card that Vulkan can handle all
    // the operations that it needs.
    for (const auto &device : devices)
    {
      if (isDeviceSuitable(device))
      {
        physicalDevice = device;
        break;
      }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
      throw std::runtime_error("Error: Failed to find a suitable GPU.\n");
    }
  }

  void createLogicalDevice()
  {
    // Specifies the number of queues we want for a single queue family.
    // We gonna specify it just to be a queue which supports graphics capabilities.
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    // Creating queues
    // TODO: In future is a good idea to make this multithread
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

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
    if (VulkanDebugger::ENABLE_VALIDATION_LAYERS)
    {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
      createInfo.enabledLayerCount = 0;
    }

    // TODO: Here we can specify extensions to do more cool stuff with vulkan.

    // Instantiate the logical.
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
      throw std::runtime_error("Error: Logical Device creation has failed!\n");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
  }

  void recreateSwapChain() 
  {
    int width = 0, height = 0;
    glfwGetFramebufferSize(this->window->getGlfwWindow(), &width, &height);
    while (width == 0 || height == 0) {
      glfwGetFramebufferSize(this->window->getGlfwWindow(), &width, &height);
      glfwWaitEvents();
    }
    vkDeviceWaitIdle(device);

    this->swapChain->recreateSwapChain(physicalDevice, device, surface, window.get());
  }

  void createCommandPool()
  {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
      throw std::runtime_error("Error: Command Pool creation has failed.\n");
    }
  }

  void createCommandBuffers()
  {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    // Allocate command buffers
    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
      throw std::runtime_error("Error: Failed to allocate Command buffers.\n");
    }
  }

  /**
   * @brief Function that writes the commands we want to execute into a command buffer.
   *
   * @param commandBuffer
   * @param imageIndex
   */
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
  {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
      throw std::runtime_error("Error: Failed to begin recording command buffer.");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass  = this->swapChain->getRenderPass();
    renderPassInfo.framebuffer = swapChain->swapChainFramebuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->swapChainExtent;

    VkClearValue clearColor        = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues    = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind vertex buffer.
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

    // Bind Graphics Pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width  = static_cast<float>(swapChain->swapChainExtent.width);
    viewport.height = static_cast<float>(swapChain->swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain->swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {pipeline->getVertexBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets); // Bind vertex buffers to bindings.

    vkCmdBindIndexBuffer(commandBuffer, pipeline->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16); // Bind index buffers.
                                                                    // Or VK_INDEX_TYPE_UINT32
                                                                    // depending the type of the
                                                                    // indices.

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(pipeline->getIndices().size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    // Finished recording the command buffer.
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
      throw std::runtime_error("Error: Failed to record Command Buffer.\n");
    }
  }

  void drawFrame()
  {
    // Wait until the previous frame has finished.
    vkWaitForFences(device, 1, &(swapChain->inFlightFences[swapChain->currentFrame]), VK_TRUE, UINT64_MAX);

    // Acquire an image from the swap chain.
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain->swapChain, UINT64_MAX, swapChain->imageAvailableSemaphores[swapChain->currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) // Means that the window has been rezised and now we have to recreate the swapchain
    {
      recreateSwapChain();
      return;
    } 
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
      throw std::runtime_error("Error: Failed to acquire swap chain image.");
    }

    // Only reset the fence if we are submitting work.
    vkResetFences(device, 1, &(swapChain->inFlightFences[swapChain->currentFrame]));

    vkResetCommandBuffer(commandBuffers[swapChain->currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(commandBuffers[swapChain->currentFrame], imageIndex);

    // Queue submission and synchronization.
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {swapChain->imageAvailableSemaphores[swapChain->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[swapChain->currentFrame];

    VkSemaphore signalSemaphores[] = {swapChain->renderFinishedSemaphores[swapChain->currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, swapChain->inFlightFences[swapChain->currentFrame]) != VK_SUCCESS)
    {
      throw std::runtime_error("Error: Failed to submit draw command buffer.");
    }

    // Submit the result back to the swap chain.
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain->swapChain};
    presentInfo.swapchainCount  = 1;
    presentInfo.pSwapchains     = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    // Submit the request to present an image to the swap chain.
    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->framebufferResized) 
    {
      window->framebufferResized = false;
      recreateSwapChain();
    } 
    else if (result != VK_SUCCESS) 
    {
      throw std::runtime_error("failed to present swap chain image!");
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
  bool isDeviceSuitable(VkPhysicalDevice device)
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
    if (extensionsSupported)
    {
      SwapChainSupportDetails swapChainSupport = swapChain->querySwapChainSupport(device, surface);
      swapChainAdequate = !swapChainSupport.surfaceFormats.empty() && !swapChainSupport.presentModes.empty();
    }

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
  bool checkDeviceExtensionSupport(VkPhysicalDevice device)
  {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
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
  std::vector<const char *> getRequiredExtensions()
  {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (VulkanDebugger::ENABLE_VALIDATION_LAYERS)
    {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }

  bool checkValidationLayerSupport()
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
};

int main()
{
  HelloTriangleApplication app;

  try
  {
    app.run();
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
