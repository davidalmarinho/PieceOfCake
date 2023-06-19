#include <stdexcept>
#include <iostream>
#include "Window.hpp"

Window::Window(const char *t_title, int t_width, int t_height, bool vsync)
{
	this->m_title  = t_title;
	this->m_width  = t_width;
	this->m_height = t_height;

	this->vsync = vsync;
}

Window::Window(const char *t_title, int t_width, int t_height) : Window(t_title, t_width, t_height, true)
{
	// Delegate constructor
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

	this->setVsync(this->vsync);

	// Create window
	this->m_glfwWindow = glfwCreateWindow(this->m_width, this->m_height, 
				   this->m_title, nullptr, nullptr);
	glfwSetWindowUserPointer(this->m_glfwWindow, this);
	glfwSetFramebufferSizeCallback(this->m_glfwWindow, this->framebufferResizeCallback);
}

void Window::createSurface(VkInstance vkInstance, VkSurfaceKHR* vkSurfaceKHR)
{
	if (glfwCreateWindowSurface(vkInstance, this->m_glfwWindow, nullptr, vkSurfaceKHR) != VK_SUCCESS) {
		// TODO: Create a cool error system handling
		throw std::runtime_error("Error: Failed to create window surface!\n");
	}
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  app->framebufferResized = true;
}

GLFWwindow *Window::getGlfwWindow()
{
	return this->m_glfwWindow;
}

bool Window::isReadyToClose()
{
	return glfwWindowShouldClose(this->m_glfwWindow);
}

bool Window::isVsyncEnabled()
{
	return this->vsync;
}

void Window::setVsync(bool vsync)
{
	this->vsync = vsync;

	// Vulkan is the entity reponsible for vsync, but is a good idea
	// to also tell glfw if vsync is desired or not.
	vsync == true ? glfwSwapInterval(1) : glfwSwapInterval(0);
}