#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

#include "DescriptorLayout.hpp"

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

  // Cache
  VkDevice cachedDevice;
  VkRenderPass cachedRenderPass;
  
  VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &code);
  // Multisample configuration
  VkPipelineMultisampleStateCreateInfo setupMultisample();
  // Stages:
  VkPipelineRasterizationStateCreateInfo setupRasterizationStage();

public:
  Pipeline(VkDevice device, VkRenderPass renderPass);
  ~Pipeline();

  void createGraphicsPipeline(VkDevice device, VkFormat swapChainImageFormat, VkRenderPass renderPass);
  void bind(VkCommandBuffer commandBuffer);

  // Getters and Setters

  VkPipeline getGraphicsPipeline();
  VkPipelineLayout getPipelineLayout();
  const std::unique_ptr<DescriptorLayout> &getDescriptorLayout() const;
};