#define GLFW_INCLUDE_VULKAN

#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <string.h>

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
        VkInstance m_vkInstance;
        GLFWwindow *m_glfwWindow;
        const u_int32_t m_WIDTH  = 800;
        const u_int32_t m_HEIGHT = 600;

        void initWindow()
        {
            glfwInit();

            // Glfw, by default, creates an OpenGL context
            // So we need to tell him that we don't want an OpenGL context
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

            // Resizable window
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            
            m_glfwWindow = 
                glfwCreateWindow(m_WIDTH, m_HEIGHT, "Vulkan", nullptr, nullptr);
        }

        void createInstance()
        {
            VkApplicationInfo appInfo{};
            appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName   = "Triangle in Vulkan :)";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName        = "PieceOfCake";
            appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion         = VK_API_VERSION_1_0;

            VkInstanceCreateInfo createInfo{};
            createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            // Connect Vulkan to Glfw
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = 
                glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            createInfo.enabledExtensionCount = glfwExtensionCount;
            createInfo.ppEnabledExtensionNames = glfwExtensions;
            createInfo.enabledLayerCount = 0;

            // Create the instance
            VkResult result = vkCreateInstance(&createInfo, nullptr, &m_vkInstance);
            
            // Check if vkInstance could be created
            if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS) {
                throw std::runtime_error("Error: Couldn't create VkInstance");
            }

            // Count how many extensions we have and put them into an array
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, 
                                                   nullptr);
            std::vector<VkExtensionProperties> extensionsVec(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                                   extensionsVec.data());

            // List vulkan available extensions
            // std::cout << "Available extensions:\n";
            // for (const auto &extension : extensionsVec) {
            //     std::cout << '\t' << extension.extensionName << '\n';
            // }

            // std::cout << "Avaadwfafilable extensions:\n";

            // Check if all glfw extensions are supported by Vulkan
            for (int i = 0; i < glfwExtensionCount - 1; i++) {
                const char *curExtension = *glfwExtensions;

                bool foundExtension = false;
                for (const auto &extension : extensionsVec) {
                    if (strcmp(curExtension, extension.extensionName))
                        foundExtension = true;
                }

                if (!foundExtension) {
                    std::cout << "Error: The glfw extension '" << curExtension 
                        << "' couldn't been supported by Vulkan.\n";
                }
                // std::cout << '\t' << *curExtension << '\n';
            }
        }

        void initVulkan() 
        {
            createInstance();
        }

        void mainLoop() 
        {
            while(!glfwWindowShouldClose(m_glfwWindow)) {
                glfwPollEvents();
            }
        }

        void cleanup() 
        {
            vkDestroyInstance(m_vkInstance, nullptr);
            glfwDestroyWindow(m_glfwWindow);
            glfwTerminate();
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
