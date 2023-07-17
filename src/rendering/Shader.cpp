#include "Shader.hpp"
#include "AssetPool.hpp"
#include "Engine.hpp"
#include "Utils.hpp"

Shader::Shader(VkDevice device, const std::string fragmentShaderFilepath, const std::string vertexShaderFilepath) : 
  cachedDevice(device), fragmentShaderFilepath(fragmentShaderFilepath), vertexShaderFilepath(vertexShaderFilepath)
{
  auto vertShaderCode = AssetPool::readFile(vertexShaderFilepath);
  auto fragShaderCode = AssetPool::readFile(fragmentShaderFilepath);
}

Shader::~Shader()
{
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(cachedDevice, uniformBuffers[i], nullptr);
    vkFreeMemory(cachedDevice, uniformBuffersMemory[i], nullptr);
  }
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

void Shader::updateUniformBuffer(uint32_t currentFrame)
{
  // TODO: This test calculates a time that should be proveninet of the frame rate.
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  UniformBufferObject ubo{};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f), 
                              Engine::get()->getRenderer()->getSwapChain()->getSwapChainExtent().width / 
                              static_cast<float>(Engine::get()->getRenderer()->getSwapChain()->getSwapChainExtent().height), 
                              0.1f, 10.0f);

  ubo.proj[1][1] *= -1;

  memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

void Shader::createUniformBuffers()
{
  VkDeviceSize bufferSize = sizeof(Shader::UniformBufferObject);

  uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
  uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  VkDevice device = Engine::get()->getRenderer()->getDevice();
  VkPhysicalDevice physicalDevice = Engine::get()->getRenderer()->getPhysicalDevice();

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    Utils::createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                 uniformBuffers[i], uniformBuffersMemory[i], device, physicalDevice);

    // Map the buffer right after creation using vkMapMemory to get a pointer to 
    // which we can write the data later on.
    // This technique is called "persistent mapping".
    vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
  }
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

const std::vector<VkBuffer> Shader::getUniformBuffers()
{
  return this->uniformBuffers;
}