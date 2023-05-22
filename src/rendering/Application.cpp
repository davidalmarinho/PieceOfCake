#include <cstdint>
#include <cstdlib>
#include <vector>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <vulkan/vulkan_core.h>
#include <set>

#include "Application.hpp"
#include "FamilyQueues.hpp"
#include "SwapChain.hpp"
#include "AssetPool.hpp"

#ifdef NDEBUG
Application::Application() : m_validationLayers(1, "VK_LAYER_KHRONOS_validation"), 
							 ENABLE_VALIDATION_LAYERS(false),
							 m_deviceExtensions(1, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
{
	// Constructor if we are in release version
}
#else
Application::Application() : m_validationLayers(1, "VK_LAYER_KHRONOS_validation"), 
							 ENABLE_VALIDATION_LAYERS(true),
							 m_deviceExtensions(1, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
{
	// Consttructor if we are in debug version
}
#endif

Application::~Application()
{
	if (this->ENABLE_VALIDATION_LAYERS) {
		destroyDebugUtilsMessengerEXT(this->m_vkInstance, 
								this->m_debugMessenger, nullptr);
	}

	vkDestroyDevice(this->m_device, nullptr);
	vkDestroySurfaceKHR(this->m_vkInstance, this->m_vkSurface, nullptr);
	vkDestroyInstance(this->m_vkInstance, nullptr);
}

void Application::vkCreateInfo()
{
	if (this->ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
		throw std::runtime_error("Validation layers requested, but not available!\n");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = "Triangle in Vulkan :)";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName        = "PieceOfCake";
	appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion         = VK_API_VERSION_1_0;
	
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Connect Vulkan to Glfw
	auto glfwExtensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
	uint32_t glfwExtensionCount = glfwExtensions.size();
	createInfo.ppEnabledExtensionNames = glfwExtensions.data();

	VkDebugUtilsMessengerCreateInfoEXT dbCreateInfo{};
	// If validation layers are enabled we want to include them
	if (this->ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(this->m_validationLayers.size());
		createInfo.ppEnabledLayerNames = this->m_validationLayers.data();

		populateDebugMessengerCreateInfo(dbCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &dbCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// Check if the instance can be created and actully create the instance
	if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS) {
		throw std::runtime_error("Error: Couldn't create VkInstance");
	}
}

void Application::createGraphicsPipeline(VkRenderPass renderPass)
{
	auto vertShaderCode = AssetPool::readFile("shaders/triangle_vertex_shader.spv");
	auto fragShaderCode = AssetPool::readFile("shaders/triangle_fragment_shader.spv");

	// Create shaders' modules
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; // Tells Vulkan in which pipeline stage the shader is going to be used.
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; 
	// Defines entry point
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = nullptr; // Shader filled with just constant
	
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	// Vextex input
	// For now, will be specified that there is no vertex data to load.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount   = 0;
	vertexInputInfo.pVertexBindingDescriptions      = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions    = nullptr;
	
	// Input Assembly
	// Input Assembly escribes two things: what kind of geometry will be 
	// drawn from the vertices and if primitive restart should be enabled.
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE; // Reuse vertices? EBO
	
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount  = 1;

	// Rasterizer --The rasterizer takes the geometry that is shaped by the 
	// vertices from the vertex shader and turns it into fragments to be 
	// colored by the fragment shader.
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable        = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode             = VK_POLYGON_MODE_FILL; // Determines how fragments are generated for geometry.
	rasterizer.lineWidth               = 1.0f;
	rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable         = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; 
	rasterizer.depthBiasClamp          = 0.0f;
	rasterizer.depthBiasSlopeFactor    = 0.0f;

	// Multisampling --is one of the ways to perform anti-aliasing.
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable   = VK_FALSE;
	multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading      = 1.0f;
	multisampling.pSampleMask           = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable      = VK_FALSE;

	// Color blending --After a fragment shader has returned a color, it needs 
	// to be combined with the color that is already in the framebuffer.
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	// Configuration for color blending use
	colorBlendAttachment.blendEnable         = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

	// This second struct references the array of structures for all of the 
	// framebuffers and allows you to set blend constants that you can use as 
	// blend factors in the aforementioned calculations.
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable     = VK_FALSE;
	colorBlending.logicOp           = VK_LOGIC_OP_COPY; 
	colorBlending.attachmentCount   = 1;
	colorBlending.pAttachments      = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// Dynamic State
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

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
	if (vkCreatePipelineLayout(
		this->m_device, &pipelineLayoutInfo, nullptr, &(this->m_pipelineLayout
		))
		!= VK_SUCCESS) {
		throw std::runtime_error("Error: Failed to create pipeline layout.");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages    = shaderStages;

	pipelineInfo.pVertexInputState   = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState      = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState   = &multisampling;
	pipelineInfo.pDepthStencilState  = nullptr;
	pipelineInfo.pColorBlendState    = &colorBlending;
	pipelineInfo.pDynamicState       = &dynamicState;

	pipelineInfo.layout = this->m_pipelineLayout;

	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass    = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(this->m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &(this->m_graphicsPipeline)) != VK_SUCCESS) {
    throw std::runtime_error("Error: Failed to create graphics pipeline.\n");
}

	// Clean shaders' modules
	vkDestroyShaderModule(this->m_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(this->m_device, vertShaderModule, nullptr);
}

bool Application::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Check if the required layers are available
	for (const char* layerName : this->m_validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			// std::cout << "Available Layer: " << layerName << "\n";
			// std::cout << "Required Layer: " << layerProperties.layerName << "\n";

			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}

// ------------------ Physical devices setup ------------------
void Application::pickPhysicalDevice()
{
	// List graphics card
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->m_vkInstance, &deviceCount, nullptr);

	// Throw error if we haven't find graphics card
	if (deviceCount == 0) {
		throw std::runtime_error("Error: Failed to find GPUs with Vulkan support.\n");
	}

	std::vector<VkPhysicalDevice> devicesVec(deviceCount);
	vkEnumeratePhysicalDevices(this->m_vkInstance, &deviceCount, devicesVec.data());

	// Check if the computer has a graphics card that Vulkan can handle all
	// the operations that it needs.
	for (const auto& device : devicesVec) {
		if (isDeviceSuitable(device)) {
			this->m_physicalDevice = device;
			break;
		}
	}
	if (this->m_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("Error: Failed to find a suitable GPU.\n");
	}

}

bool Application::isDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	// Get device properties so we can check if the graphics cards 
	// support geometry shaders, in this case
	/*
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(this->m_physicalDevice, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(this->m_physicalDevice, &deviceFeatures);

	return deviceProperties.deviceType == 
		VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
	*/

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, this->m_vkSurface);
	
	bool extensionSupport = checkDeviceExtensionSupport(physicalDevice);

	bool swapChainAdequate = false;
	if (extensionSupport) {
		SwapChainSupportDetails swapChainSupport 
			= querySwapChainSupport(physicalDevice, this->m_vkSurface); 
		swapChainAdequate = !swapChainSupport.surfaceFormats.empty() 
							&& !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionSupport && swapChainAdequate;
}


void Application::createLogicalDevice()
{
	// Specifies the number of queues we want for a single queue family.
	// We gonna specify it just to be a queue which supports graphics capabilities.
	QueueFamilyIndices indices = findQueueFamilies(this->m_physicalDevice, this->m_vkSurface);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoVec;
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	// Creating queues
	// TODO: In future is a good idea to make this multithread
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount       = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfoVec.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	// Creating Logical Device.
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	// Pointers to the queue creation info and device features structs
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoVec.size());
	createInfo.pQueueCreateInfos    = queueCreateInfoVec.data();
	createInfo.pEnabledFeatures     = &deviceFeatures;

	// Turn on swap chain system.
	createInfo.enabledExtensionCount   = static_cast<uint32_t>(this->m_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = this->m_deviceExtensions.data();

	// Guarantee compatibility with older devices and older vulkan devices.
	// Because this isn't needed anymore.
	if (this->ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount = 
			static_cast<uint32_t>(this->m_validationLayers.size());
		createInfo.ppEnabledLayerNames = this->m_validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	
	// TODO: Here we can specify extensions to do more cool stuff with vulkan.
	
	// Instantiate the logical.
	if (vkCreateDevice(this->m_physicalDevice, &createInfo, 
					nullptr, &this->m_device) != VK_SUCCESS) {
		throw std::runtime_error("Error: Logical Device creation has failed!\n");
	}

	// Retrieve queue handles for each queue family
	vkGetDeviceQueue(this->m_device, indices.graphicsFamily.value(), 0, &this->m_graphicsQueue);
	vkGetDeviceQueue(this->m_device, indices.presentFamily.value(), 0,
			   &this->m_presentQueue);
}
// ------------------ End Physical Devices setup ------------------------

// ------------------ Vulkan Messenger Debugger setup ------------------

VKAPI_ATTR VkBool32 VKAPI_CALL Application::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		// TODO: Message is important enough to show
	}

	return VK_FALSE;
}

VkResult createDebugUtilsMessengerEXT(VkInstance vkInstance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
	
	if (func != nullptr)
		return func(vkInstance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Application::setupDebugMessenger()
{
	if (!this->ENABLE_VALIDATION_LAYERS) return;

	// Setup messenger's callbacks
	VkDebugUtilsMessengerCreateInfoEXT dbCreateInfo{};
	populateDebugMessengerCreateInfo(dbCreateInfo);

	if (createDebugUtilsMessengerEXT
		(this->m_vkInstance, &dbCreateInfo, nullptr, &this->m_debugMessenger) 
		    != VK_SUCCESS) {
		throw std::runtime_error("Error: Failed to set up debug messenger.\n");
	}
}

void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& dbCreateInfo)
{
	dbCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	dbCreateInfo.messageSeverity = 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	dbCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	
	dbCreateInfo.pfnUserCallback = Application::debugCallback;
	// dbCreateInfo.pUserData = nullptr;
}

/**
 * @brief Get extensions which are required by vulkan, like:
 *		  -> VK_KHR_surface
 *
 * @return 
 */
std::vector<const char*> Application::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	std::vector<const char*> extensionsVec(
			glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (this->ENABLE_VALIDATION_LAYERS) {
		extensionsVec.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensionsVec;
}

void Application::destroyDebugUtilsMessengerEXT(
	VkInstance vkInstance, 
	VkDebugUtilsMessengerEXT dbMessenger, 
	const VkAllocationCallbacks *pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
				vkGetInstanceProcAddr(this->m_vkInstance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(vkInstance, dbMessenger, pAllocator);
	}
}
// ------------------ End Vulkan Messenger Debugger setup ------------------

// Swapchain

/**
 * @brief Checks if the device has a graphics cards which can handle with
 *        the swap chain mechaninsm. So, we need a graphics card capable
 *        of drawing images and act as FrameBuffer 
 *        --When an image is been draw, we 1 or more images prepared to
 *          be called for rendering.
 *
 * @param vkDevice Vulkan physical device.
 * @return true if there is a graphics card capable of handling swap chain mechanism.
 */
bool Application::checkDeviceExtensionSupport(VkPhysicalDevice vkDevice)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(vkDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensionsVec(extensionCount);
	vkEnumerateDeviceExtensionProperties(vkDevice, nullptr, &extensionCount, availableExtensionsVec.data());

	std::set<std::string> requiredExtensions(this->m_deviceExtensions.begin(),
							this->m_deviceExtensions.end());

	for (const auto& extension : availableExtensionsVec) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}


// ------------------ Shaders modules setup ------------------
/**
 * @brief Create VkShaderModule object so the program is able to pass
 * shaders' code to the pipeline.
 *
 * @param code 
 * @return 
 */
VkShaderModule Application::createShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    // Create VkShaderModule
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(this->m_device, &createInfo, 
			     nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module!\n");
    }

    return shaderModule;
}
// ------------------ End Shaders modules setup ------------------

// Getters and setters
VkInstance Application::getVkInstance()
{
	return this->m_vkInstance;
}

VkDevice Application::getDevice()
{
	return this->m_device;
}

VkPhysicalDevice Application::getPhysicalDevice()
{
	return this->m_physicalDevice;
}

VkSurfaceKHR* Application::getVkSurfacePtr()
{
	return &this->m_vkSurface;
}

VkSurfaceKHR Application::getVkSurface()
{
	return this->m_vkSurface;
}

VkPipelineLayout Application::getPipelineLayout()
{
	return this->m_pipelineLayout;
}

VkPipeline Application::getGraphicsPipeline()
{
	return this->m_graphicsPipeline;
}

VkQueue Application::getGraphicsQueue()
{
	return this->m_graphicsQueue;
}

VkQueue Application::getPresentQueue()
{
	return this->m_presentQueue;
}

