#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stdexcept>
#include "stb_image.h"
#include "Utils.hpp"

Texture::Texture(VkDevice device, const std::string filepath) : cachedDevice(device), filepath(filepath)
{

}

Texture::~Texture()
{
  vkDestroySampler(cachedDevice, textureSampler, nullptr);
  vkDestroyImageView(cachedDevice, textureImageView, nullptr);

  vkDestroyImage(cachedDevice, textureImage, nullptr);
  vkFreeMemory(cachedDevice, textureImageMemory, nullptr);
}

void Texture::createTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, 
                                 VkQueue graphicsQueue, VkCommandPool commandPool)
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    throw std::runtime_error("Error: Failed to load texture image: '" + this->filepath + "'.\n");
  }

  // Create a buffer in host visible memory so that we can use vkMapMemory and copy the pixels to it.
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  Utils::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
               stagingBuffer, stagingBufferMemory, device, physicalDevice);

  // Copy the pixel values that we got from the image loading library to the buffer.
  void* data;
  vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
      memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(device, stagingBufferMemory);

  // Clean up the original pixel array.
  stbi_image_free(pixels);

  Utils::createImage(device, physicalDevice, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, 
              VK_IMAGE_TILING_OPTIMAL, 
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

  // Prepare the texture image.
  Utils::transitionImageLayout(device, graphicsQueue, commandPool, 
                        textureImage, VK_FORMAT_R8G8B8A8_SRGB, 
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  copyBufferToImage(device, graphicsQueue, commandPool, 
                    stagingBuffer, textureImage, 
                    static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  Utils::transitionImageLayout(device, graphicsQueue, commandPool, textureImage, 
                        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Texture::createTextureImageView(VkDevice device)
{
  this->textureImageView = Utils::createImageView(device, textureImage, 
                                                  VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Texture::createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice)
{
  // Specify all filters and transformations that should be applied.
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  // Specify how to interpolate texels that are magnified or minified.
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;

  // NOTE: Here exists the possibility to play around with textures.
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  // Specify if anisotropic filtering should be used.
  samplerInfo.anisotropyEnable = VK_TRUE;
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  // Limits the amount of texel samples that can be used to calculate the final color.
  // In this case it is being choosen quality over performance.
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  /*
   * NOTE:
   * To disable anisotropic filtering:
   * samplerInfo.anisotropyEnable = VK_FALSE;
   * samplerInfo.maxAnisotropy = 1.0f;
   */

  // Specify which color is returned when sampling beyond the image with clamp to border addressing mode.
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

  // Specify which coordinate system you want to use to address texels in an image.
  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  // If a comparison function is enabled, then texels will first be compared to 
  // a value, and the result of that comparison is used in filtering operations. 
  // This is mainly used for percentage-closer filtering on shadow maps.
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp     = VK_COMPARE_OP_ALWAYS;

  // Mipmapping configuration.
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create the texture sampler.\n");
  }
}

void Texture::copyBufferToImage(VkDevice device, VkQueue graphicsQueue, 
                                VkCommandPool commandPool,
                                VkBuffer buffer, VkImage image, 
                                uint32_t width, uint32_t height)
{
  VkCommandBuffer commandBuffer = Utils::beginSingleTimeCommands(device, commandPool);

  // Specifies which part of the buffer is going to be copied to which part of the image.
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {
      width,
      height,
      1
  };

  vkCmdCopyBufferToImage(
    commandBuffer,
    buffer,
    image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &region
  );

  Utils::endSingleTimeCommands(device, graphicsQueue, commandPool, commandBuffer);
}

// Getters and Setters

VkImageView Texture::getTextureImageView()
{
  return this->textureImageView;
}

VkSampler Texture::getTextureSampler()
{
  return this->textureSampler;
}

const std::string Texture::getFilepath()
{
  return this->filepath;
}