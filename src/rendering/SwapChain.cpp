#include "SwapChain.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <limits>
#include <algorithm>
#include <GLFW/glfw3.h>

SwapChainSupportDetails querySwapChainSupport(
	VkPhysicalDevice vkDevice, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;
	
	// -- Setting up surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkDevice, surface, &details.capabilities);
	
	// -- Setting up supported surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice, surface, &formatCount, nullptr);
	// Resize vector to hold wanted changes
	if (formatCount != 0) {
		details.surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice, surface, 
				&formatCount, details.surfaceFormats.data());
	}

	// -- Setting up presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(vkDevice, surface, 
				    &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(vkDevice, surface,
				&presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) {
		// Pick SRGB color format if it is available
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;	
		}
	}

	// If there isn't any available format which we are looking for, we are
	// going to just pick the first we have found
	// TODO: Warning message
	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(
	const std::vector<VkPresentModeKHR> &availablePresentMode)
{
	/**
	 * Existing presentation modes ↓
	 * VK_PRESENT_MODE_IMMEDIATE_KHR:
	 *		The only presentation mode which is guaranted to have.
	 *		Renders the image asap.
	 *		Causes screen tearing.
	 *	VK_PRESENT_MODE_FIFO_KHR:
	 *		Basically vsync.
	 *		Lower power consumation
	 *	VK_PRESENT_MODE_FIFO_RELAXED_KHR:
	 *		Similar to previous. Instead of waiting for the next vertical 
	 *		blank, the image is transferred right away when it finally arrives. 
	 *		This may result in visible tearing.
	 *	VK_PRESENT_MODE_MAILBOX_KHR:
	 *		Triple buffering.
	 *		Avoids tearing and is used to render frames asap.
	 *		Higher power consume
	 */

	for (const auto& availablePresentMode : availablePresentMode) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(GLFWwindow *glfwWindow, 
							 const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width 
			!= std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(glfwWindow, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		// Bound the values of width and height between the allowed minimum 
		// and maximum extents that are supported by the implementation.
		actualExtent.width = std::clamp(actualExtent.width,
								  capabilities.minImageExtent.width,
								  capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height,
								   capabilities.minImageExtent.height,
								   capabilities.maxImageExtent.width);

		return actualExtent;
	}
}

