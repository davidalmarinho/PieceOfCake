#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>
#include <glm/gtx/hash.hpp>
#include <string>

class Model
{
public:
  // Structure to specify an array of vertex data.
  struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoords;
    glm::vec3 normalCoords;

    // Tell Vulkan how to pass this data format to the vertex shader once it's been uploaded into GPU memory. 
    static VkVertexInputBindingDescription getBindingDescription();

    // Describe how to extract a vertex attribute from a chunk of vertex data 
    // originating from a binding description. We have three attributes: position;
    // color and texture coordinates. So we need three attribute description structs.
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();

    bool operator==(const Vertex& other) const;
  };

  Model(const std::string FILEPATH, const std::vector<Vertex> &vertices, std::vector<uint32_t> indices);
  ~Model();
  void init();

  const std::string FILEPATH;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

  // Getters and Setters

  VkBuffer getVertexBuffer();
  VkDeviceMemory getVertexBufferMemory();
  VkBuffer getIndexBuffer();
  VkDeviceMemory getIndexBufferMemory();
  uint32_t getIndicesCount();

private:
  std::vector<Vertex> vertices;  
  std::vector<uint32_t> indices;

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
  void createIndexBuffer(const std::vector<uint32_t> indices);
};

namespace std {
  template<> struct hash<Model::Vertex> {
    size_t operator()(Model::Vertex const& vertex) const {
      return ((hash<glm::vec3>()(vertex.pos) ^
        (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
        (hash<glm::vec2>()(vertex.texCoords) << 1);
    }
  };
}