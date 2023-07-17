#include "Model.hpp"
#include "Engine.hpp"
#include "DescriptorLayout.hpp"
#include "Utils.hpp"

#include <memory>
#include <cstring>
#include <stdexcept>

Model::Model(const std::vector<Vertex> &vertices,  std::vector<uint16_t> indices) : cachedDevice(Engine::get()->getRenderer()->getDevice())
{
  this->createVertexBuffer(vertices);
  this->createIndexBuffer(indices);
  this->indicesCount = indices.size();
}

Model::~Model()
{
  vkDestroyBuffer(cachedDevice, indexBuffer, nullptr);
  vkFreeMemory(cachedDevice, indexBufferMemory, nullptr);

  vkDestroyBuffer(cachedDevice, vertexBuffer, nullptr);
  vkFreeMemory(cachedDevice, vertexBufferMemory, nullptr);
}

void Model::createVertexBuffer(const std::vector<Vertex> &vertices)
{
  VkDevice device = Engine::get()->getRenderer()->getDevice();
  VkPhysicalDevice physicalDevice = Engine::get()->getRenderer()->getPhysicalDevice();
  VkCommandPool commandPool = Engine::get()->getRenderer()->getCommandPool();
  VkQueue graphicsQueue = Engine::get()->getRenderer()->getGraphicsQueue();

  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  // Create staging buffer
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  Utils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
               stagingBuffer, stagingBufferMemory, device, physicalDevice);

  // Copy the vertex data to the buffer.
  void* data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(device, stagingBufferMemory);

  Utils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, 
               vertexBufferMemory, device, physicalDevice);

  Utils::copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, vertexBuffer, bufferSize);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Model::createIndexBuffer(const std::vector<uint16_t> indices)
{
  VkDevice device  = Engine::get()->getRenderer()->getDevice();
  VkPhysicalDevice physicalDevice = Engine::get()->getRenderer()->getPhysicalDevice();
  VkCommandPool commandPool = Engine::get()->getRenderer()->getCommandPool();
  VkQueue graphicsQueue = Engine::get()->getRenderer()->getGraphicsQueue();

  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  
  Utils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
               stagingBuffer, stagingBufferMemory, 
               device, physicalDevice);

  void* data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), (size_t) bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  Utils::createBuffer(bufferSize, 
               VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory, 
               device, physicalDevice);

  Utils::copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

VkVertexInputBindingDescription Model::Vertex::getBindingDescription()
{
  VkVertexInputBindingDescription bindingDescription{};

  bindingDescription.binding   = 0; // Specify the index of the binding in the array of bindings.
  bindingDescription.stride    = sizeof(Vertex); // Specify the number of bytes from one entry to the next.
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move to the next data entry after each vertex.

  return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Model::Vertex::getAttributeDescriptions()
{
  std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

  // Position attribute.
  attributeDescriptions[0].binding  = 0; // Tell Vulkan from which binding the per-vertex data comes.
  attributeDescriptions[0].location = 0; // References the location directive of the input in the vertex shader. 
                                         // The input in the vertex shader with location 0 is the position, which has two 32-bit float components.
  attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT; // Describe the type of data for the attribute.
  attributeDescriptions[0].offset   = offsetof(Vertex, pos); // Specify the number of bytes since the start of the per-vertex data to read from.

  // Color attribute.
  attributeDescriptions[1].binding  = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset   = offsetof(Vertex, color);

  // Texture coordinates attribute.
  attributeDescriptions[2].binding  = 0;
  attributeDescriptions[2].location = 2;
  attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[2].offset   = offsetof(Vertex, texCoords);

  return attributeDescriptions;
}

void Model::bind(VkCommandBuffer commandBuffer)
{
  VkBuffer vertexBuffers[] = {this->vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets); // Bind vertex buffers to bindings.
  vkCmdBindIndexBuffer(commandBuffer, this->indexBuffer, 0, VK_INDEX_TYPE_UINT16); // Bind index buffers.
                                                         // Or VK_INDEX_TYPE_UINT32
                                                         // depending the type of the
                                                         // indices.
}

void Model::draw(VkCommandBuffer commandBuffer)
{
  vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(this->indicesCount), 1, 0, 0, 0);
}

// Getters and Setters

VkBuffer Model::getVertexBuffer()
{
  return this->vertexBuffer;
}

VkDeviceMemory Model::getVertexBufferMemory()
{
  return this->vertexBufferMemory;
}

VkBuffer Model::getIndexBuffer()
{
  return this->indexBuffer;
}

VkDeviceMemory Model::getIndexBufferMemory()
{
  return this->indexBufferMemory;
}

uint32_t Model::getIndicesCount()
{
  return this->indicesCount;
}
