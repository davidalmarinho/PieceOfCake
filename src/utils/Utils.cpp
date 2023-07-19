#include "Utils.hpp"

#include <stdexcept>

void Utils::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                    VkMemoryPropertyFlags properties, VkBuffer& buffer, 
                    VkDeviceMemory& bufferMemory,
                    VkDevice device,
                    VkPhysicalDevice physicalDevice)
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;   // Specify the size of the buffer in bytes.
  bufferInfo.usage = usage; // Indicate for which purposes the data in the buffer is going to be used. 
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Buffers can also be 
                                                      // owned by a specific 
                                                      // queue family or be 
                                                      // shared between multiple at the same time. 
                                                      // The buffer will only be used from the graphics queue, 
                                                      // so we can stick to exclusive access.
  bufferInfo.flags = 0; //  Configure sparse buffer memory.

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create buffer.\n");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  // Memory allocation.
  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
      throw std::runtime_error("Error: Failed to allocate buffer memory.\n");
  }

  // If memory allocation was successful, then we can now associate this memory with the buffer using.
  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

/**
  * @brief Graphics cards can offer different types of memory to allocate from. 
  * Each type of memory varies in terms of allowed operations and performance 
  * characteristics. We need to combine the requirements of the buffer and our 
  * own application requirements to find the right type of memory to use.
  * So, there is a need to determine the right memory type.
  * 
  * @param typeFilter Specifies the bit field of memory types that are suitable. 
  * @param properties 
  * @return uint32_t 
  */
uint32_t Utils::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

VkCommandBuffer Utils::beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

  // Record command buffer.
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void Utils::endSingleTimeCommands(VkDevice device, VkQueue graphicsQueue, 
                                  VkCommandPool commandPool, VkCommandBuffer commandBuffer)
{
  // Stop recording / copying
  vkEndCommandBuffer(commandBuffer);

  // Complete the transfer.
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue); // Wait for the transfer queue to become idle.

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

// Copies the contents from one buffer to another,
void Utils::copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, 
                       VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size = size;

  // Transfer the contents of buffers.
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(device, graphicsQueue, commandPool, commandBuffer);
}

VkImageView Utils::createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;

  /* viewType and format fields specify how the image data should
   * be interpreted. The viewType parameter allows you to treat
   * images as 1D textures, 2D textures, 3D textures and cube maps.
   */
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;

  // The subresourceRange field describes what the image’s
  // purpose is and which part of the image should be accessed.
  viewInfo.subresourceRange.aspectMask     = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel   = 0;
  viewInfo.subresourceRange.levelCount     = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = 1;

  /*  _________________________________________________________
   * | Note:                                                   |
   * | If you were working on a stereographic 3D application,  |
   * | then you would create a swap chain with multiple layers.|
   * |_________________________________________________________|
   */

  // Create image views
  VkImageView imageView;
  if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create texture image views.\n");
  }

  return imageView;
}

void Utils::createImage(VkDevice device, VkPhysicalDevice physicalDevice,
                 uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, 
                 VkImageTiling tiling, VkImageUsageFlags usage, 
                 VkMemoryPropertyFlags properties, VkImage& image, 
                 VkDeviceMemory& imageMemory)
{

  VkImageCreateInfo imageInfo{};
  imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType     = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width  = static_cast<uint32_t>(width);
  imageInfo.extent.height = static_cast<uint32_t>(height);
  imageInfo.extent.depth  = 1;
  imageInfo.mipLevels     = mipLevels;
  imageInfo.arrayLayers   = 1;

  // Use the same format for the texels as the pixels in the buffer, otherwise the copy operation will fail.
  imageInfo.format = format;

  // VK_IMAGE_TILING_LINEAR: Texels are laid out in row-major order like our pixels array.
  // VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation defined order for optimal access.
  imageInfo.tiling = tiling;


  // VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Not usable by the GPU and the very first transition will discard the texels.

  imageInfo.usage = usage;


  // Image will only be used by one queue family: the one that supports graphics (and therefore also) transfer operations.
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags   = 0; // NOTE: To make a Voxel Engine you should explore the available flags for this option.
  
  if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create the image.\n");
  }

  // Allocate memory for the image.
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = Utils::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    physicalDevice);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  vkBindImageMemory(device, image, imageMemory, 0);
}

// Handles image layout transitions.
void Utils::transitionImageLayout(VkDevice device, VkQueue graphicsQueue, 
                                    VkCommandPool commandPool, VkImage image, 
                                    VkFormat format, VkImageLayout oldLayout, 
                                    VkImageLayout newLayout, uint32_t mipLevels)
{
  VkCommandBuffer commandBuffer = Utils::beginSingleTimeCommands(device, commandPool);

  // Use pipeline barrier like that is generally used to synchronize access to resources.
  // NOTE: We can use this barrier to transfer queue family ownership when VK_SHARING_MODE_EXCLUSIVE is used. 
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;

  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  // Specifies the image that is affected and the specific part of the image.
  barrier.image = image;

  if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (Utils::hasStencilComponent(format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } 
  else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  barrier.subresourceRange.baseMipLevel   = 0;
  barrier.subresourceRange.levelCount     = mipLevels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount     = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  // Transfer destination: transfer writes that don't need to wait on anything.
  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  // Shader reading: shader reads should wait on transfer writes, 
  // specifically the shader reads in the fragment shader, because that's where 
  // we're going to use the texture.
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }
  else {
    throw std::invalid_argument("Error: Unsupported layout transition.\n");
  }

  vkCmdPipelineBarrier(
    commandBuffer,
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );

  Utils::endSingleTimeCommands(device, graphicsQueue, commandPool, commandBuffer);
}

// Helper function that tells us if the chosen depth format contains a stencil component.
bool Utils::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}