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
  
  VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

  void createImage(VkDevice device, VkPhysicalDevice physicalDevice,
                   uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels, 
                   VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
                   VkMemoryPropertyFlags properties, VkImage& image, 
                   VkDeviceMemory& imageMemory);

  void transitionImageLayout(VkDevice device, VkQueue graphicsQueue, 
                             VkCommandPool commandPool, VkImage image, 
                             VkFormat format, VkImageLayout oldLayout, 
                             VkImageLayout newLayout, uint32_t mipLevels);

  bool hasStencilComponent(VkFormat format);
}