#include "Pipeline.hpp"
#include "AssetPool.hpp"

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

// Structure to specify an array of vertex data.
struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  // Tell Vulkan how to pass this data format to the vertex shader once it's been uploaded into GPU memory. 
  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};

    bindingDescription.binding   = 0; // Specify the index of the binding in the array of bindings.
    bindingDescription.stride    = sizeof(Vertex); // Specify the number of bytes from one entry to the next.
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move to the next data entry after each vertex.

    return bindingDescription;
  }

  // Describe how to extract a vertex attribute from a chunk of vertex data 
  // originating from a binding description. We have two attributes, position 
  // and color, so we need two attribute description structs.
  static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

    // Position attribute
    attributeDescriptions[0].binding  = 0; // Tell Vulkan from which binding the per-vertex data comes.
    attributeDescriptions[0].location = 0; // References the location directive of the input in the vertex shader. 
                                           // The input in the vertex shader with location 0 is the position, which has two 32-bit float components.
    attributeDescriptions[0].format   = VK_FORMAT_R32G32_SFLOAT; // Describe the type of data for the attribute.
    attributeDescriptions[0].offset   = offsetof(Vertex, pos); // Specify the number of bytes since the start of the per-vertex data to read from.

    // Color attribute
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
  }
};

const std::vector<Vertex> vertices = {
  //  X      Y       R     G     B
  {{-0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}}, // Top Left Corner
  {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, // Top Right Corner
  {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}, // Bottom Right Corner
  {{-0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}}  // Bottom Left Corner
};

Pipeline::Pipeline(VkDevice device) : cachedDevice(device)
{

}

Pipeline::~Pipeline()
{
  vkDestroyBuffer(cachedDevice, indexBuffer, nullptr);
  vkFreeMemory(cachedDevice, indexBufferMemory, nullptr);

  vkDestroyBuffer(cachedDevice, vertexBuffer, nullptr);
  vkFreeMemory(cachedDevice, vertexBufferMemory, nullptr);

  vkDestroyPipeline(cachedDevice, graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(cachedDevice, pipelineLayout, nullptr);

  vkDestroyRenderPass(cachedDevice, renderPass, nullptr);
}

// Shaders modules setup
/**
 * @brief Create VkShaderModule object so the program is able to pass
 * shaders' code to the pipeline.
 *
 * @param code
 * @return
 */
VkShaderModule Pipeline::createShaderModule(VkDevice device, const std::vector<char> &code)
{
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode    = reinterpret_cast<const uint32_t *>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
  {
    throw std::runtime_error("Error: Failed to create shader module.\n");
  }

  return shaderModule;
}

void Pipeline::createGraphicsPipeline(VkDevice device, VkFormat swapChainImageFormat)
{
  this->createRenderPass(device, swapChainImageFormat);

  auto vertShaderCode = AssetPool::readFile("shaders/triangle_vertex_shader.spv");
  auto fragShaderCode = AssetPool::readFile("shaders/triangle_fragment_shader.spv");

  // Create shaders' modules
  VkShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
  VkShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);

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
  // For now, will be specified that there is no vertex data to load.
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = nullptr;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr;

  // Set up the graphics pipeline to accept vertex data.
  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();
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
  pipelineLayoutInfo.setLayoutCount         = 0;
  pipelineLayoutInfo.pSetLayouts            = nullptr;
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
  pipelineInfo.pDepthStencilState  = nullptr;
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

void Pipeline::createRenderPass(VkDevice device, VkFormat swapChainImageFormat)
{
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format  = swapChainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

  colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;     // Specifies which layout the image will have before the render pass begins.
  colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Specifies the layout to automatically transition to when the render pass finishes.

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0; // Specifies which attachment to reference by its index in the attachment descriptions array.
  colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1; // The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive.
  subpass.pColorAttachments    = &colorAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;

  dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;

  dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments    = &colorAttachment;
  renderPassInfo.subpassCount    = 1;
  renderPassInfo.pSubpasses      = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies   = &dependency;

  if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create the render pass.\n");
  }
}

void Pipeline::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                    VkMemoryPropertyFlags properties, VkBuffer& buffer, 
                    VkDeviceMemory& bufferMemory,
                    VkDevice device,
                    VkPhysicalDevice physicalDevice)
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size; // Specify the size of the buffer in bytes.
  bufferInfo.usage = usage; // Indicate for which purposes the data in the buffer is going to be used. 
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Buffers can also be 
                                                      // owned by a specific 
                                                      // queue family or be 
                                                      // shared between multiple at the same time. 
                                                      // The buffer will only be used from the graphics queue, 
                                                      // so we can stick to exclusive access.
  bufferInfo.flags = 0; //  Configure sparse buffer memory.

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create buffer.\n");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  // Memory allocation.
  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate buffer memory!");
  }

  // If memory allocation was successful, then we can now associate this memory with the buffer using.
  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Pipeline::createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                                  VkCommandPool commandPool, VkQueue graphicsQueue)
{
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  // Create staging buffer
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  this->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
               stagingBuffer, stagingBufferMemory, device, physicalDevice);

  // Copy the vertex data to the buffer.
  void* data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(device, stagingBufferMemory);

  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, 
               vertexBufferMemory, device, physicalDevice);

  copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, vertexBuffer, bufferSize);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Pipeline::createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                                 VkCommandPool commandPool, VkQueue graphicsQueue)
{
  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
               stagingBuffer, stagingBufferMemory, 
               device, physicalDevice);

  void* data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), (size_t) bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  createBuffer(bufferSize, 
               VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory, 
               device, physicalDevice);

  copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

// Copies the contents from one buffer to another,
void Pipeline::copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, 
                          VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

  // Record command buffer.
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size = size;

  // Transfer the contents of buffers.
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  // Stop recording / copying
  vkEndCommandBuffer(commandBuffer);

  // Complete the transfer.
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue); // Wait for the transfer queue to become idle.

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
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

/**
  * @brief Graphics cards can offer different types of memory to allocate from. 
  * Each type of memory varies in terms of allowed operations and performance 
  * characteristics. We need to combine the requirements of the buffer and our 
  * own application requirements to find the right type of memory to use.
  * So, there is a need to determine the right memory type.
  * 
  * @param typeFilter Specifies the bit field of memory types that are suitable. 
  * @param properties 
  * @return uint32_t 
  */
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
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
  rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;

  rasterizer.depthBiasEnable         = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp          = 0.0f;
  rasterizer.depthBiasSlopeFactor    = 0.0f;

  return rasterizer;
}

std::vector<uint16_t> Pipeline::getIndices()
{
  return this->indices;
}

VkPipelineLayout Pipeline::getPipelineLayout()
{
  return this->pipelineLayout;
}

VkPipeline Pipeline::getGraphicsPipeline()
{
  return this->graphicsPipeline;
}

VkBuffer Pipeline::getVertexBuffer()
{
  return this->vertexBuffer;
}

VkBuffer Pipeline::getIndexBuffer()
{
  return this->indexBuffer;
}

VkRenderPass Pipeline::getRenderPass()
{
  return this->renderPass;
}