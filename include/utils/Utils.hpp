#pragma once

#include <vulkan/vulkan.h>

namespace Utils
{
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                    VkMemoryPropertyFlags properties, VkBuffer& buffer, 
                    VkDeviceMemory& bufferMemory,
                    VkDevice device,
                    VkPhysicalDevice physicalDevice);
  
  void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, 
                          VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
  void endSingleTimeCommands(VkDevice device, VkQueue graphicsQueue, 
                                  VkCommandPool commandPool, VkCommandBuffer commandBuffer);
  
  VkImageView createImageView(VkDevice device, VkImage image, VkFormat format);
}