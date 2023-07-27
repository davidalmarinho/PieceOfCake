#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

#include "DescriptorLayout.hpp"

class DescriptorLayout;
class ColorBlending
{
public:
  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  VkPipelineColorBlendStateCreateInfo colorBlending = {};

  ColorBlending();
  ~ColorBlending();
};

class Pipeline 
{
private:
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;
  std::unique_ptr<DescriptorLayout> descriptorLayout;

  // For uniform buffers --shaders' global constants.
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;

  // Cache
  VkDevice cachedDevice;
  VkRenderPass cachedRenderPass;

  // Multisample configuration
  VkPipelineMultisampleStateCreateInfo setupMultisample(VkSampleCountFlagBits msaaSamples);
  // Stages:
  VkPipelineRasterizationStateCreateInfo setupRasterizationStage();

public:
  struct UniformBufferObject {
    // Alignas to allign the data with multiples of 64.
    // For example: glm::mat4 is 64 bits, so we do alignas (16 bytes)
    //              glm::vec2 is 32 bits, so we do alignas (8 bytes)
    alignas (16) glm::mat4 model;
    alignas (16) glm::mat4 view;
    alignas (16) glm::mat4 proj;
  };

  Pipeline(VkDevice device, VkRenderPass renderPass);
  ~Pipeline();

  void createGraphicsPipeline(VkDevice device, VkFormat swapChainImageFormat, 
                              VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples);
  void bind(VkCommandBuffer commandBuffer);

  void updateUniformBuffer(uint32_t currentFrame, int modelRendererIndex);
  void createUniformBuffers();

  // Getters and Setters

  VkPipeline getGraphicsPipeline();
  VkPipelineLayout getPipelineLayout();
  const std::unique_ptr<DescriptorLayout> &getDescriptorLayout() const;
  const std::vector<VkBuffer> getUniformBuffers();
};