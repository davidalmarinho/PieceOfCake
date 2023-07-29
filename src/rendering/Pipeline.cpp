#include "Pipeline.hpp"
#include "AssetPool.hpp"
#include "Model.hpp"
#include "Engine.hpp"

#include <array>
#include <glm/glm.hpp>
#include <stdexcept>
#include <cstring>
#include <memory>

#include "PerspectiveCamera.hpp"
#include "Transform.hpp"
#include "Utils.hpp"

ColorBlending::ColorBlending()
{
  // Colorblending setup.
  // Color blending --After a fragment shader has returned a color, it needs
  // to be combined with the color that is already in the framebuffer.
  this->colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  this->colorBlendAttachment.blendEnable = VK_FALSE;

  // Configuration for color blending use
  this->colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  this->colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  this->colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
  this->colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  this->colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  this->colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

  // This second struct references the array of structures for all of the
  // framebuffers and allows you to set blend constants that you can use as
  // blend factors in the aforementioned calculations.
  this->colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  this->colorBlending.logicOpEnable   = VK_FALSE;
  this->colorBlending.logicOp         = VK_LOGIC_OP_COPY;
  this->colorBlending.attachmentCount = 1;
  this->colorBlending.pAttachments    = &(this->colorBlendAttachment);
  this->colorBlending.blendConstants[0] = 0.0f;
  this->colorBlending.blendConstants[1] = 0.0f;
  this->colorBlending.blendConstants[2] = 0.0f;
  this->colorBlending.blendConstants[3] = 0.0f;
}

ColorBlending::~ColorBlending(){}

Pipeline::Pipeline(VkDevice device, VkRenderPass renderPass) : cachedDevice(device), cachedRenderPass(renderPass)
{
  this->descriptorLayout = std::make_unique<DescriptorLayout>(device);
}

Pipeline::~Pipeline()
{
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(cachedDevice, uniformBuffers[i], nullptr);
    vkFreeMemory(cachedDevice, uniformBuffersMemory[i], nullptr);
  }

  vkDestroyPipeline(cachedDevice, graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(cachedDevice, pipelineLayout, nullptr);

  this->descriptorLayout.reset();
}

void Pipeline::bind(VkCommandBuffer commandBuffer)
{
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);
}

void Pipeline::createGraphicsPipeline(VkDevice device, VkFormat swapChainImageFormat, 
                                      VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples)
{
  std::shared_ptr shader = AssetPool::getShader("texture");

  // Create shaders' modules
  VkShaderModule fragShaderModule = shader->compile(device, shader->getFragmentShaderCode());
  VkShaderModule vertShaderModule = shader->compile(device, shader->getVertexShaderCode());

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; // Tells Vulkan in which pipeline stage the shader is going to be used.
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  // Defines entry point
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName  = "main";
  vertShaderStageInfo.pSpecializationInfo = nullptr; // Shader filled with just constant

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName  = "main";
  fragShaderStageInfo.pSpecializationInfo = nullptr;

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  // Vextex input
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = nullptr;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr;

  // Set up the graphics pipeline to accept vertex data.
  auto bindingDescription = Model::Vertex::getBindingDescription();
  auto attributeDescriptions = Model::Vertex::getAttributeDescriptions();
  vertexInputInfo.vertexBindingDescriptionCount   = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

  // 01 Stage - Input Assembly
  // Input Assembly escribes two things: what kind of geometry will be
  // drawn from the vertices and if primitive restart should be enabled.
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE; // Reuse vertices? EBO

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer = this->setupRasterizationStage();
  
  // Multisampling --is one of the ways to perform anti-aliasing.
  VkPipelineMultisampleStateCreateInfo multisampling = this->setupMultisample(msaaSamples);

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable       = VK_TRUE;
  depthStencil.depthWriteEnable      = VK_TRUE;
  depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds        = 0.0f;
  depthStencil.maxDepthBounds        = 1.0f;
  depthStencil.stencilTestEnable     = VK_FALSE;
  depthStencil.front                 = {};
  depthStencil.back                  = {};

  ColorBlending colorBlending = ColorBlending();

  // Dynamic State
  std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount         = 1;
  pipelineLayoutInfo.pSetLayouts            = this->descriptorLayout->getDescriptorSetLayoutPointer();
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges    = nullptr;

  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create pipeline layout.\n");
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;

  pipelineInfo.pVertexInputState   = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState      = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState   = &multisampling;
  pipelineInfo.pDepthStencilState  = &depthStencil;
  pipelineInfo.pColorBlendState    = &(colorBlending.colorBlending);
  pipelineInfo.pDynamicState       = &dynamicState;

  pipelineInfo.layout = pipelineLayout;

  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;

  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create graphics pipeline.\n");
  }

  // Clean shaders' modules
  vkDestroyShaderModule(device, fragShaderModule, nullptr);
  vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

VkPipelineMultisampleStateCreateInfo Pipeline::setupMultisample(VkSampleCountFlagBits msaaSamples)
{
  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.rasterizationSamples  = msaaSamples;
  multisampling.pSampleMask           = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable      = VK_FALSE;

  if (Engine::get()->getRenderer()->sampleShading) {
    multisampling.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
    multisampling.minSampleShading    = .2f; // min fraction for sample shading; closer to one is smoother
  }
  else {
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.minSampleShading    = 1.0f;
  }

  return multisampling;
}

// Setup existing stages in Pipeline class.

/**
 * @brief 5th Stage: Rasterization --The rasterizer takes the geometry 
 * that is shaped by the vertices from the vertex shader and turns it into 
 * fragments to be colored by the fragment shader.
 * In this function it will be setup a "VkPipelineRasterizationStateCreateInfo"
 * to handle this job.
 * 
 * @return VkPipelineRasterizationStateCreateInfo returns the configured
 *         "VkPipelineRasterizationStateCreateInfo".
 */
VkPipelineRasterizationStateCreateInfo Pipeline::setupRasterizationStage()
{
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

  /* If true, forces gl_Position to be less than zero and clamp it to zero or to 
   * be greater than one and clamp it to one. So it is a setting that is usually 
   * set to false.
   * NOTE: To enable it to true is also needed a GPU feature:
   *       VKPipelineRasterizationDepthClipStateCreateInfoEXT.
   */
  rasterizer.depthClampEnable        = VK_FALSE;

  // If true, primitives are discarded immediatly befora the rasterization stage.
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  // Determines how fragments are generated for geometry.
  rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;

  rasterizer.lineWidth               = 1.0f;

  // Tell if the indices will be followed up by counter-clockwise mode
  // or followed up by clockwise mode.
  rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;

  rasterizer.depthBiasEnable         = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp          = 0.0f;
  rasterizer.depthBiasSlopeFactor    = 0.0f;

  return rasterizer;
}

void Pipeline::updateUniformBuffer(uint32_t currentFrame, int modelRendererIndex)
{
  UniformBufferObject ubo{};

  if (Engine::get()->getRenderer()->getEntity(modelRendererIndex)) {
    const Entity& e = Engine::get()->getRenderer()->getEntity(modelRendererIndex).value();
    ubo.model = e.getComponent<Transform>().getModelMatrix();
  }
  else {
    ubo.model = glm::translate(glm::vec3(0, 0, 0));
  }

  ubo.view = Engine::get()->getCamera().getComponent<PerspectiveCamera>().getViewMatrix();
  ubo.proj = glm::perspective(glm::radians(Engine::get()->getCamera().getComponent<PerspectiveCamera>().getFoV()), 
                              Engine::get()->getRenderer()->getSwapChain()->getSwapChainExtent().width / 
                              static_cast<float>(Engine::get()->getRenderer()->getSwapChain()->getSwapChainExtent().height), 
                              Engine::get()->getCamera().getComponent<PerspectiveCamera>().zNear, 
                              Engine::get()->getCamera().getComponent<PerspectiveCamera>().zFar);

  if (Engine::get()->getRenderer()->getEntity(modelRendererIndex)) {
    const Entity& e = Engine::get()->getRenderer()->getEntity(modelRendererIndex).value();
    ubo.normalMatrix = e.getComponent<Transform>().getNormalMatrix();
  }
  else {
    ubo.normalMatrix = glm::identity<glm::mat3>();
  }

  ubo.proj[1][1] *= -1;

  memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

void Pipeline::createUniformBuffers()
{
  VkDeviceSize bufferSize = sizeof(Pipeline::UniformBufferObject);

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

VkPipelineLayout Pipeline::getPipelineLayout()
{
  return this->pipelineLayout;
}

VkPipeline Pipeline::getGraphicsPipeline()
{
  return this->graphicsPipeline;
}

const std::unique_ptr<DescriptorLayout> &Pipeline::getDescriptorLayout() const
{
  return this->descriptorLayout;
}

const std::vector<VkBuffer> Pipeline::getUniformBuffers()
{
  return this->uniformBuffers;
}