#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vulkan/vulkan.hpp>
#include <string>

// TODO: Remove / Edit this includes when possible.
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
// ------------------------------------------------

class Shader
{
public:
  struct UniformBufferObject {
    // Alignas to allign the data with multiples of 64.
    // For example: glm::mat4 is 64 bits, so we do alignas (16 bytes)
    //              glm::vec2 is 32 bits, so we do alignas (8 bytes)
    alignas (16) glm::mat4 model;
    alignas (16) glm::mat4 view;
    alignas (16) glm::mat4 proj;
  };

  Shader(VkDevice device, const std::string fragmentShaderFilepath, const std::string vertexShaderFilepath);
  ~Shader();

  VkShaderModule compile(VkDevice device, const std::vector<char> &code);
  void createUniformBuffers();
  void updateUniformBuffer(uint32_t currentFrame);

  // Getters and Setters
  const std::vector<char> getFragmentShaderCode();
  const std::vector<char> getVertexShaderCode();
  const std::string getFragmentShaderFilepath();
  const std::string getVertexShaderFilepath();
  const std::vector<VkBuffer> getUniformBuffers();

private:
  const std::string fragmentShaderFilepath;
  const std::string vertexShaderFilepath;

  // For uniform buffers --shaders' global constants.
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;

  // Cache.
  VkDevice cachedDevice;
};