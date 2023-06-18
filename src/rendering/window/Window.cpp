#include <stdexcept>

#include "Window.hpp"

Window::Window(const char *t_title, int t_width, int t_height)
{
	this->m_title  = t_title;
	this->m_width  = t_width;
	this->m_height = t_height;
}

Window::~Window() {
	glfwDestroyWindow(this->m_glfwWindow);
	glfwTerminate();
}

void Window::init()
{
	glfwInit();

	// GLFW, by default, creates an OpenGL context. So, we need to tell it that
	// we don't want an OpenGL context, since we are working with Vulkan
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// Create window
	this->m_glfwWindow = glfwCreateWindow(this->m_width, this->m_height, 
				   this->m_title, nullptr, nullptr);
}

void Window::createSurface(VkInstance vkInstance, VkSurfaceKHR* vkSurfaceKHR)
{
	if (glfwCreateWindowSurface(vkInstance, this->m_glfwWindow, nullptr, vkSurfaceKHR) != VK_SUCCESS) {
		// TODO: Create a cool error system handling
		throw std::runtime_error("Error: Failed to create window surface!\n");
	}
}

GLFWwindow *Window::getGlfwWindow()
{
	return this->m_glfwWindow;
}

bool Window::isReadyToClose()
{
	return glfwWindowShouldClose(this->m_glfwWindow);
}