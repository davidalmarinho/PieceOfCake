#pragma once

#include <vulkan/vulkan.hpp>

class Texture
{
private:
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  
  // Cache
  VkDevice cachedDevice;

  void createImage(VkDevice device, VkPhysicalDevice physicalDevice,
                 uint32_t texWidth, uint32_t texHeight, VkFormat format, 
                 VkImageTiling tiling, VkImageUsageFlags usage, 
                 VkMemoryPropertyFlags properties, VkImage& image, 
                 VkDeviceMemory& imageMemory);

  void transitionImageLayout(VkDevice device, VkQueue graphicsQueue, 
                             VkCommandPool commandPool, VkImage image, 
                             VkFormat format, VkImageLayout oldLayout, 
                             VkImageLayout newLayout);
  
  void copyBufferToImage(VkDevice device, VkQueue graphicsQueue, 
                                VkCommandPool commandPool,
                                VkBuffer buffer, VkImage image, 
                                uint32_t width, uint32_t height);

public:
  Texture(VkDevice device);
  ~Texture();
  void createTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, 
                                 VkQueue graphicsQueue, VkCommandPool commandPool);
};