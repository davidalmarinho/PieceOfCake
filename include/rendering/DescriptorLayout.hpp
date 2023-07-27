#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Texture.hpp"
#include "Pipeline.hpp"

class Pipeline;
class DescriptorLayout
{
public:
  DescriptorLayout(VkDevice device);
  ~DescriptorLayout();

  void createDescriptorPool();
  void createDescriptorSets(Pipeline* pipeline, Texture* texture);
  void bind(Pipeline* pipeline, VkCommandBuffer commandBuffer);

  // Getters and Setters

  VkDescriptorSetLayout getDescriptorSetLayout();
  const VkDescriptorSetLayout *getDescriptorSetLayoutPointer();

private:
  std::vector<VkDescriptorSet> descriptorSets;

  VkDescriptorPool descriptorPool;
  VkDescriptorSetLayout descriptorSetLayout;

  void createDescriptorSetLayout(VkDevice device);

  // Cache
  VkDevice cachedDevice;
};