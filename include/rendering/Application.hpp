#pragma once

#include <vulkan/vulkan_core.h>

class Application
{
	private:
		VkInstance m_vkInstance;

	public:
		Application();
		~Application();
		void vkCreateInfo();
};
