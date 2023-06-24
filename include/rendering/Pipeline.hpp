#pragma once

#include <vulkan/vulkan.h>
#include <vector>

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

class Pipeline 
{
private:
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  // For vertex buffers
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  // For indices
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;

  // Cache
  VkDevice cachedDevice;

  const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
  };

public:

  Pipeline(VkDevice device);
  ~Pipeline();

  VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &code);
  void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass);
  void createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                         VkCommandPool commandPool, VkRenderPass renderPass, VkQueue graphicsQueue);
  void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                          VkCommandPool commandPool, VkQueue graphicsQueue);
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                    VkMemoryPropertyFlags properties, VkBuffer& buffer, 
                    VkDeviceMemory& bufferMemory,
                    VkDevice device,
                    VkPhysicalDevice physicalDevice);
  void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue,
                  VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  void destroy(VkDevice device);

  std::vector<uint16_t> getIndices();
  VkPipeline getGraphicsPipeline();
  VkPipelineLayout getPipelineLayout();
  VkBuffer getVertexBuffer();
  VkBuffer getIndexBuffer();
};