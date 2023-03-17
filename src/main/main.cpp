#define GLFW_INCLUDE_VULKAN

#include <iostream>

#include "Window.hpp"
#include "Application.hpp"

class HelloTriangleApplication 
{
    public:
        void run() 
        {
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }

    private:
        Window *window;
        Application *application;
        const u_int32_t m_WIDTH  = 800;
        const u_int32_t m_HEIGHT = 600;

        void initWindow()
        {
            window = new Window("PieceOfCake", 800, 600);
            window->init();
        }

        void createInstance()
        {
            application = new Application();
            application->vkCreateInfo();
        }

        void initVulkan() 
        {
            createInstance();
        }

        void mainLoop() 
        {
            while(!glfwWindowShouldClose(window->getGlfwWindow())) {
                glfwPollEvents();
            }
        }

        void cleanup() 
        {
            delete application;
            delete window;
        }
    };

    int main(int argc, char *argv[])
    {
        HelloTriangleApplication app;
        app.run();

        try {
        } catch(const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
