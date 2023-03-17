#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

class Application
{
	private:
		VkInstance m_vkInstance;
		const std::vector<const char*> m_validationLayers;
		const bool ENABLE_VALIDATION_LAYERS;

	public:
		Application();
		~Application();
		void vkCreateInfo();
		bool checkValidationLayerSupport();
};
