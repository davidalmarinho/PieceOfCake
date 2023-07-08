#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

class DescriptorLayout
{
public:
  struct UniformBufferObject {
    // Alignas to allign the data with multiples of 64.
    // For example: glm::mat4 is 64 bits, so we do alignas (16 bytes)
    //              glm::vec2 is 32 bits, so we do alignas (8 bytes)
    alignas (16) glm::mat4 model;
    alignas (16) glm::mat4 view;
    alignas (16) glm::mat4 proj;
  };

  DescriptorLayout(VkDevice device);
  ~DescriptorLayout();

  void createDescriptorPool();
  void createDescriptorSets();
  void createUniformBuffers();
  void updateUniformBuffer(uint32_t currentFrame);
  void bind(VkCommandBuffer commandBuffer);

  // Getters and Setters

  VkDescriptorSetLayout getDescriptorSetLayout();
  const VkDescriptorSetLayout *getDescriptorSetLayoutPointer();

private:
  VkDescriptorSetLayout descriptorSetLayout;
  std::vector<VkDescriptorSet> descriptorSets;

  VkDescriptorPool descriptorPool;

  // For uniform buffers --shaders' global constants
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;

  void createDescriptorSetLayout(VkDevice device);

  // Cache
  VkDevice cachedDevice;
};