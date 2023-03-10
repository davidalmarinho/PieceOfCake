#include "Window.hpp"
#include <GLFW/glfw3.h>

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

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Create window
	this->m_glfwWindow = glfwCreateWindow(this->m_width, this->m_height, 
				   this->m_title, nullptr, nullptr);
}

GLFWwindow *Window::getGlfwWindow()
{
	return this->m_glfwWindow;
}

