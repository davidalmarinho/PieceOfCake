#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vulkan/vulkan.hpp>
#include <string>

class Shader
{
public:
  Shader(VkDevice device, const std::string fragmentShaderFilepath, const std::string vertexShaderFilepath);
  ~Shader();

  VkShaderModule compile(VkDevice device, const std::vector<char> &code);

  // Getters and Setters
  const std::vector<char> getFragmentShaderCode();
  const std::vector<char> getVertexShaderCode();
  const std::string getFragmentShaderFilepath();
  const std::string getVertexShaderFilepath();

private:
  const std::string fragmentShaderFilepath;
  const std::string vertexShaderFilepath;

  // Cache.
  VkDevice cachedDevice;
};