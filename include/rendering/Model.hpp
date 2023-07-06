#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

class Model
{
public:
  // Structure to specify an array of vertex data.
  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    // Tell Vulkan how to pass this data format to the vertex shader once it's been uploaded into GPU memory. 
    static VkVertexInputBindingDescription getBindingDescription();

    // Describe how to extract a vertex attribute from a chunk of vertex data 
    // originating from a binding description. We have two attributes, position 
    // and color, so we need two attribute description structs.
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
  };

  Model(const std::vector<Vertex> &vertices, std::vector<uint16_t> indices);
  ~Model();

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

  // Getters and Setters

  VkBuffer getVertexBuffer();
  VkDeviceMemory getVertexBufferMemory();
  VkBuffer getIndexBuffer();
  VkDeviceMemory getIndexBufferMemory();
  uint32_t getIndicesCount();

private:
  // For vertex buffers
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  // For indices
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;

  uint32_t indicesCount;

  // Cache
  VkDevice cachedDevice;

  void createVertexBuffer(const std::vector<Model::Vertex> &vertices);
  void createIndexBuffer(const std::vector<uint16_t> indices);
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                    VkMemoryPropertyFlags properties, VkBuffer& buffer, 
                    VkDeviceMemory& bufferMemory,
                    VkDevice device,
                    VkPhysicalDevice physicalDevice);
  void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue,
                  VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);
};