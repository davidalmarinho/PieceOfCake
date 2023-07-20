#pragma once

#include <vulkan/vulkan.hpp>

class Texture
{
private:
  const std::string filepath;
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;

  // Mipmaping config.
  uint32_t mipLevels;

  // Cache
  VkDevice cachedDevice;
  
  void copyBufferToImage(VkDevice device, VkQueue graphicsQueue, 
                                VkCommandPool commandPool,
                                VkBuffer buffer, VkImage image, 
                                uint32_t width, uint32_t height);

  void generateMipmaps(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool commandPool,
                       VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

public:
  Texture(VkDevice device, const std::string filepath);
  ~Texture();
  void createTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, 
                                 VkQueue graphicsQueue, VkCommandPool commandPool);
  void createTextureImageView(VkDevice device);
  void createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice);

  void clean(VkDevice device);

  // Getters and Setters

  VkImageView getTextureImageView();
  VkSampler getTextureSampler();
  const std::string getFilepath();
};