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
  bufferInfo.size = size; // Specify the size of the buffer in bytes.
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