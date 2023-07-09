#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include "Shader.hpp"

class DescriptorLayout
{
public:
  DescriptorLayout(VkDevice device);
  ~DescriptorLayout();

  void createDescriptorPool();
  void createDescriptorSets(Shader* shader);
  void bind(VkCommandBuffer commandBuffer);

  // Getters and Setters

  VkDescriptorSetLayout getDescriptorSetLayout();
  const VkDescriptorSetLayout *getDescriptorSetLayoutPointer();

private:
  VkDescriptorSetLayout descriptorSetLayout;
  std::vector<VkDescriptorSet> descriptorSets;

  VkDescriptorPool descriptorPool;

  void createDescriptorSetLayout(VkDevice device);

  // Cache
  VkDevice cachedDevice;
};