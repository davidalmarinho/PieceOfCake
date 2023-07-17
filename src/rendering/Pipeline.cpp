#include "Pipeline.hpp"
#include "AssetPool.hpp"
#include "Model.hpp"

#include <array>
#include <glm/glm.hpp>
#include <stdexcept>
#include <cstring>
#include <memory>

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
  vkDestroyPipeline(cachedDevice, graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(cachedDevice, pipelineLayout, nullptr);

  vkDestroyRenderPass(cachedDevice, cachedRenderPass, nullptr);

  this->descriptorLayout.reset();
}

void Pipeline::bind(VkCommandBuffer commandBuffer)
{
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);
}

void Pipeline::createGraphicsPipeline(VkDevice device, VkFormat swapChainImageFormat, VkRenderPass renderPass)
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
  VkPipelineMultisampleStateCreateInfo multisampling = this->setupMultisample();

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

VkPipelineMultisampleStateCreateInfo Pipeline::setupMultisample()
{
  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable   = VK_FALSE;
  multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading      = 1.0f;
  multisampling.pSampleMask           = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable      = VK_FALSE;

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