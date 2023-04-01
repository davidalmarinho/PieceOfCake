#include "FamilyQueues.hpp"

bool QueueFamilyIndices::isComplete()
{
	return this->graphicsFamily.has_value();
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
	// Logic to find graphics queue family
	QueueFamilyIndices indices;
	
	// Get the list of queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamiliesVec(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 
					  queueFamiliesVec.data());

	// Find queue family which supports VK_QUEUE_GRAPHICS_BIT
	int i = 0;
	for (const auto& queueFamily : queueFamiliesVec) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		// Check if we have already found all the queue families
		// that we require
		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

