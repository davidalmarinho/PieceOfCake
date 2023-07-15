#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stdexcept>
#include "stb_image.h"
#include "Utils.hpp"

Texture::Texture(VkDevice device) : cachedDevice(device)
{

}

Texture::~Texture()
{
  vkDestroyImage(cachedDevice, textureImage, nullptr);
  vkFreeMemory(cachedDevice, textureImageMemory, nullptr);
}

void Texture::createImage(VkDevice device, VkPhysicalDevice physicalDevice,
                 uint32_t texWidth, uint32_t texHeight, VkFormat format, 
                 VkImageTiling tiling, VkImageUsageFlags usage, 
                 VkMemoryPropertyFlags properties, VkImage& image, 
                 VkDeviceMemory& imageMemory)
{

  VkImageCreateInfo imageInfo{};
  imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType     = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width  = static_cast<uint32_t>(texWidth);
  imageInfo.extent.height = static_cast<uint32_t>(texHeight);
  imageInfo.extent.depth  = 1;
  imageInfo.mipLevels     = 1;
  imageInfo.arrayLayers   = 1;

  // Use the same format for the texels as the pixels in the buffer, otherwise the copy operation will fail.
  imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;

  // VK_IMAGE_TILING_LINEAR: Texels are laid out in row-major order like our pixels array.
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // Texels are laid out in an implementation defined order for optimal access.


  // VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Not usable by the GPU and the very first transition will discard the texels.

  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;


  // Image will only be used by one queue family: the one that supports graphics (and therefore also) transfer operations.
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags   = 0; // NOTE: To make a Voxel Engine you should explore the available flags for this option.
  
  if (vkCreateImage(device, &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create the image.\n");
  }

  // Allocate memory for the image.
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device, textureImage, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = Utils::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    physicalDevice);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  vkBindImageMemory(device, textureImage, textureImageMemory, 0);
}

void Texture::createTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, 
                                 VkQueue graphicsQueue, VkCommandPool commandPool)
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load("assets/img.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    throw std::runtime_error("Error: Failed to load texture image.\n");
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

  createImage(device, physicalDevice, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, 
              VK_IMAGE_TILING_OPTIMAL, 
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

  // Prepare the texture image.
  transitionImageLayout(device, graphicsQueue, commandPool, 
                        textureImage, VK_FORMAT_R8G8B8A8_SRGB, 
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  copyBufferToImage(device, graphicsQueue, commandPool, 
                    stagingBuffer, textureImage, 
                    static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  transitionImageLayout(device, graphicsQueue, commandPool, textureImage, 
                        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

// Handles image layout transitions.
void Texture::transitionImageLayout(VkDevice device, VkQueue graphicsQueue, 
                                    VkCommandPool commandPool, VkImage image, 
                                    VkFormat format, VkImageLayout oldLayout, 
                                    VkImageLayout newLayout)
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
  barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel   = 0;
  barrier.subresourceRange.levelCount     = 1;
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
