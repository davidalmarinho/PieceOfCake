#pragma once

#include <vulkan/vulkan.hpp>

class Texture
{
private:
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;
  
  // Cache
  VkDevice cachedDevice;
  
  void copyBufferToImage(VkDevice device, VkQueue graphicsQueue, 
                                VkCommandPool commandPool,
                                VkBuffer buffer, VkImage image, 
                                uint32_t width, uint32_t height);

public:
  Texture(VkDevice device);
  ~Texture();
  void createTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, 
                                 VkQueue graphicsQueue, VkCommandPool commandPool);
  void createTextureImageView(VkDevice device);
  void createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice);

  // Getters and Setters

  VkImageView getTextureImageView();
  VkSampler getTextureSampler();
};