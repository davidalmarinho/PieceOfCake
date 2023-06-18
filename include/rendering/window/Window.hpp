#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <vulkan/vulkan.h>

class Window
{
	private:
		GLFWwindow *m_glfwWindow;
		const char* m_title;
		uint32_t m_width  = 800;
		uint32_t m_height = 600;

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	public:
		bool framebufferResized = false;

		Window(const char *title, int width, int height);
		~Window();
		void init();
		void createSurface(VkInstance vkInstance, VkSurfaceKHR* vkSurfaceKHR);

		GLFWwindow *getGlfwWindow();
		bool isReadyToClose();
};
