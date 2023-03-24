#pragma once

#include <vulkan/vulkan_core.h>
#include <iostream>
#include <vector>
#include <optional>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;

	bool isComplete();
};

/**
 * Check which Queue Families are available and supported by the device.
 * 
 * @param device
 * @return 
 */
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
