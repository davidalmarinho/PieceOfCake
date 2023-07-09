#include "DescriptorLayout.hpp"
#include "Engine.hpp"
#include "Utils.hpp"

#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

DescriptorLayout::DescriptorLayout(VkDevice device) : cachedDevice(device)
{
  this->createDescriptorSetLayout(device);
}

DescriptorLayout::~DescriptorLayout()
{
  AssetPool::cleanup();
  vkDestroyDescriptorPool(cachedDevice, descriptorPool, nullptr);

  vkDestroyDescriptorSetLayout(cachedDevice, descriptorSetLayout, nullptr);
}

void DescriptorLayout::createDescriptorSetLayout(VkDevice device)
{
  // Specify binding type.
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;

  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1; // Number of values in the array.

  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Specify in which shader stages the descriptor is going to be referenced.
  uboLayoutBinding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &uboLayoutBinding;

  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create descriptor set layout.\n");
  }
}

void DescriptorLayout::createDescriptorPool()
{
  // Describe which descriptor types our descriptor sets are going to contain 
  // and how many of them, using VkDescriptorPoolSize structures.
  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.flags = 0;

  // Specify the maximum number of descriptor sets that may be allocated.
  poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  if (vkCreateDescriptorPool(Engine::get()->getRenderer()->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create descriptor pool.\n");
  }
}

void DescriptorLayout::createDescriptorSets(Shader* shader)
{
  // Create one descriptor set for each frame in flight, all with the same layout.
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  allocInfo.pSetLayouts = layouts.data();

  descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(Engine::get()->getRenderer()->getDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to allocate descriptor sets.\n");
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    // Specify the buffer and the region within it that contains the data for the descriptor.
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = shader->getUniformBuffers()[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(Shader::UniformBufferObject);

    // Update configuration of descriptors.
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSets[i];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(Engine::get()->getRenderer()->getDevice(), 1, &descriptorWrite, 0, nullptr);
  }
}

void DescriptorLayout::bind(VkCommandBuffer commandBuffer)
{
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          Engine::get()->getRenderer()->getPipeline()->getPipelineLayout(), 0, 1,
                          &(descriptorSets[Engine::get()->getRenderer()->getSwapChain()->currentFrame]), 0, nullptr);
}

// Getters and Setters

VkDescriptorSetLayout DescriptorLayout::getDescriptorSetLayout()
{
  return this->descriptorSetLayout;
}

const VkDescriptorSetLayout *DescriptorLayout::getDescriptorSetLayoutPointer()
{
  return &(this->descriptorSetLayout);
}