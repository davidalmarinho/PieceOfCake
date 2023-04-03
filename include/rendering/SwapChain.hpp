#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

struct SwapChainSupportDetails
{
	// We need to check for 3 kinds of prorperties:
	// • Basic surface capabilities (min/max number of images in swap chain,
	//   min/-max width and height of images)
	VkSurfaceCapabilitiesKHR capabilities;
	// • Surface formats (pixel format, color space)
	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	// • Available presentation modes
	std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice vkDevice,
					VkSurfaceKHR surface);

// Setting up Surface Format --depth color
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

// Setting up Presentation Mode --conditions for swaping images to the screen
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentMode);

// Setting up Swap Extent --resolution of images in swap chain
VkExtent2D chooseSwapExtent(GLFWwindow *glfwWindow, const VkSurfaceCapabilitiesKHR& capabilities);
