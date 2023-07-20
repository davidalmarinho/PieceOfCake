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
  vkDestroyDescriptorPool(cachedDevice, descriptorPool, nullptr);

  vkDestroyDescriptorSetLayout(cachedDevice, descriptorSetLayout, nullptr);
}

void DescriptorLayout::createDescriptorSetLayout(VkDevice device)
{
  // Specify binding type.
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;

  uboLayoutBinding.descriptorCount = 1; // Number of values in the array.
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Specify in which shader stages the descriptor is going to be referenced.

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // This is where the color of the fragment is going to be determined.

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings    = bindings.data();

  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create descriptor set layout.\n");
  }
}

void DescriptorLayout::createDescriptorPool()
{
  // Describe which descriptor types our descriptor sets are going to contain 
  // and how many of them, using VkDescriptorPoolSize structures.
  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes    = poolSizes.data();
  poolInfo.maxSets       = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  poolInfo.flags = 0;

  // Specify the maximum number of descriptor sets that may be allocated.
  poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  if (vkCreateDescriptorPool(Engine::get()->getRenderer()->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create descriptor pool.\n");
  }
}

void DescriptorLayout::createDescriptorSets(Shader* shader, Texture* texture)
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
    
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture->getTextureImageView();
    imageInfo.sampler = texture->getTextureSampler();

    // Update configuration of descriptors.
    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(Engine::get()->getRenderer()->getDevice(), 
                           static_cast<uint32_t>(descriptorWrites.size()), 
                           descriptorWrites.data(), 0, nullptr);
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