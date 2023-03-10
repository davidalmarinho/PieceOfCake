#pragma once

#include <GLFW/glfw3.h>
#include <cstdlib>
#include <memory>

class Window
{
	private:
		GLFWwindow *m_glfwWindow;
		const char* m_title;
		u_int32_t m_width  = 800;
		u_int32_t m_height = 600;

	public:
		Window(const char *t_title, int t_width, int t_height);
		~Window();
		void init();

		GLFWwindow *getGlfwWindow();
};
