#include "Shader.hpp"
#include "AssetPool.hpp"
#include "Engine.hpp"
#include "Utils.hpp"

#include "PerspectiveCamera.hpp"
#include "Transform.hpp"

Shader::Shader(VkDevice device, const std::string fragmentShaderFilepath, const std::string vertexShaderFilepath) : 
  cachedDevice(device), fragmentShaderFilepath(fragmentShaderFilepath), vertexShaderFilepath(vertexShaderFilepath)
{
  auto vertShaderCode = AssetPool::readFile(vertexShaderFilepath);
  auto fragShaderCode = AssetPool::readFile(fragmentShaderFilepath);
}

Shader::~Shader()
{

}

/**
 * @brief Shaders modules setup. Creates a VkShaderModule object so the program 
 * is able to pass shaders' code to the pipeline.
 *
 * @param code
 * @return
 */
VkShaderModule Shader::compile(VkDevice device, const std::vector<char> &code)
{
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode    = reinterpret_cast<const uint32_t *>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create shader module.\n");
  }

  return shaderModule;
}

// Getters and Setters

const std::vector<char> Shader::getFragmentShaderCode()
{
  return AssetPool::readFile(this->fragmentShaderFilepath);
}

const std::vector<char> Shader::getVertexShaderCode()
{
  return AssetPool::readFile(this->vertexShaderFilepath);
}

const std::string Shader::getFragmentShaderFilepath()
{
  return this->fragmentShaderFilepath;
}

const std::string Shader::getVertexShaderFilepath()
{
  return this->vertexShaderFilepath;
}