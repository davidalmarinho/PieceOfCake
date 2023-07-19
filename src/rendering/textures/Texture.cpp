#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stdexcept>
#include "stb_image.h"
#include "Utils.hpp"
#include "Engine.hpp"

Texture::Texture(VkDevice device, const std::string filepath) : cachedDevice(device), filepath(filepath)
{

}

Texture::~Texture()
{
  this->cleanTexture();
}

void Texture::cleanTexture()
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

  // Calculate the number of levels in the mip chain.
  if (Engine::get()->getRenderer()->isMipmapping)
    this->mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
  else
    this->mipLevels = 1;

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

  Utils::createImage(device, physicalDevice, texWidth, texHeight, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, 
              VK_IMAGE_TILING_OPTIMAL, 
              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

  // Prepare the texture image.
  Utils::transitionImageLayout(device, graphicsQueue, commandPool, 
                        textureImage, VK_FORMAT_R8G8B8A8_SRGB, 
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
  copyBufferToImage(device, graphicsQueue, commandPool, 
                    stagingBuffer, textureImage, 
                    static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

  // Transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps.
  if (!Engine::get()->getRenderer()->isMipmapping) {
    Utils::transitionImageLayout(device, graphicsQueue, commandPool, textureImage, 
                          VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
  }

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);

  if (Engine::get()->getRenderer()->isMipmapping) {
    generateMipmaps(device, physicalDevice, graphicsQueue, commandPool,
                    textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
  }
}

void Texture::createTextureImageView(VkDevice device)
{
  this->textureImageView = Utils::createImageView(device, textureImage, 
                                                  VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
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
  if (Engine::get()->getRenderer()->isMipmapping)
    samplerInfo.maxLod = static_cast<float>(mipLevels);
  else
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

void Texture::generateMipmaps(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool commandPool,
                              VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
  // Check if image format supports linear blitting
  VkFormatProperties formatProperties;
  vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

  if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    throw std::runtime_error("Error: Texture image format does not support linear blitting.\n");
  }

  VkCommandBuffer commandBuffer = Utils::beginSingleTimeCommands(device, commandPool);

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  int32_t mipWidth = texWidth;
  int32_t mipHeight = texHeight;

  for (uint32_t i = 1; i < mipLevels; i++) {
    // Record each of the VkCmdBlitImage commands.
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
      0, nullptr,
      0, nullptr,
      1, &barrier
    );
    
    // Specify the regions that will be used in the blit operation.
    VkImageBlit blit{};
    blit.srcOffsets[0] = { 0, 0, 0 };
    blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = { 0, 0, 0 };
    blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;

    // Record the blit command.
    vkCmdBlitImage(commandBuffer,
      image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1, &blit,
      VK_FILTER_LINEAR
    );

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
      0, nullptr,
      0, nullptr,
      1, &barrier
    );

    if (mipWidth > 1) mipWidth /= 2;
    if (mipHeight > 1) mipHeight /= 2;
  }

  barrier.subresourceRange.baseMipLevel = mipLevels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(commandBuffer,
    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
    0, nullptr,
    0, nullptr,
    1, &barrier
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