#include "FamilyQueues.hpp"
#include "SwapChain.hpp"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
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

        // TODO: Put these in another file
        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;

        void initWindow()
        {
            window = new Window("PieceOfCake", 800, 600);
            window->init();
        }

        void createInstance()
        {
            this->application = new Application();
            this->application->vkCreateInfo();
        }

        void initVulkan() 
        {
            createInstance();
            application->setupDebugMessenger();
            window->createSurface(application);
            application->pickPhysicalDevice();
            application->createLogicalDevice();
            createSwapChain();
            createImageViews();
            application->createGraphicsPipeline();
        }

        void createImageViews()
        {
            // Resize the list to fit all of the image views we’ll be creating
            swapChainImageViews.resize(swapChainImages.size());
            for (size_t i = 0; i < swapChainImages.size(); i++) {
                VkImageViewCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                createInfo.image = swapChainImages[i];

                /* viewType and format fields specify how the image data should 
                 * be interpreted. The viewType parameter allows you to treat 
                 * images as 1D textures, 2D textures, 3D textures and cube maps.
                 */
                createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                createInfo.format   = swapChainImageFormat;

                // Swizzle the image color channels
                createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

                // The subresourceRange field describes what the image’s 
                // purpose is and which part of the image should be accessed.
                createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                createInfo.subresourceRange.baseMipLevel   = 0;
                createInfo.subresourceRange.levelCount     = 1;
                createInfo.subresourceRange.baseArrayLayer = 0;
                createInfo.subresourceRange.layerCount     = 1;

                /*  _________________________________________________________
                 * | Note:                                                   |
                 * | If you were working on a stereographic 3D application,  |
                 * | then you would create a swap chain with multiple layers.|
                 * |_________________________________________________________|
                 */
                
                // Create image views
                if (vkCreateImageView(application->getDevice(), &createInfo, 
                            nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Error: Failed to create image views!\n");
                }
            }
        }

        void createSwapChain()
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(
                application->getPhysicalDevice(), application->getVkSurface());

            VkSurfaceFormatKHR surfaceFormat = 
                chooseSwapSurfaceFormat(swapChainSupport.surfaceFormats);

            VkPresentModeKHR presentMode = 
                chooseSwapPresentMode(swapChainSupport.presentModes);

            VkExtent2D extent = 
                chooseSwapExtent(window->getGlfwWindow(), swapChainSupport.capabilities);

            // Specify how many images we want in the swap chain
            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
            // +1 because it is recommended to ask for at least 1 more image
            // than the minimum.

            // Don't exceed the maximum number of images
            if (swapChainSupport.capabilities.maxImageCount > 0 
                && imageCount > swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }

            // Finally, actually creating and integrating swap chain into Vulkan
            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = application->getVkSurface();

            // Specifying swap chain details
            createInfo.minImageCount    = imageCount;
            createInfo.imageFormat      = surfaceFormat.format;
            createInfo.imageColorSpace  = surfaceFormat.colorSpace;
            createInfo.imageExtent      = extent;
            createInfo.imageArrayLayers = 1; // Always 1 unless you are developing a stereoscopic 3D application
            createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            // Here we’re going to render directly to the images, which
            // means that they’re used as color attachment. It is also possible 
            // render images to a separate image first to perform operations 
            // like post-processing.
            // In that case we may use a value like 
            // VK_IMAGE_USAGE_TRANSFER_DST_BIT instead
            // and use a memory operation to transfer the rendered image 
            // to a swap chain image. 

            QueueFamilyIndices indices = findQueueFamilies(
                application->getPhysicalDevice(), application->getVkSurface());
            uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                        indices.presentFamily.value()};

            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices   = queueFamilyIndices;
            }
            else {
                createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0;
                createInfo.pQueueFamilyIndices   = nullptr;
            }

            // • VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family
            // at a time and ownership must be explicitly transferred before using it in
            // another queue family. This option offers the best performance.
            // It is even nicer when presentation family and graphics family
            // are the same
            // • VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue
            // families without explicit ownership transfers.

            createInfo.preTransform = 
                        swapChainSupport.capabilities.currentTransform;

            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode    = presentMode;
            createInfo.clipped        = VK_TRUE; // We don’t care about the color of pixels that are obscured
            createInfo.oldSwapchain   = VK_NULL_HANDLE;
            if (vkCreateSwapchainKHR(application->getDevice(), &createInfo, 
                                     nullptr, &swapChain) != VK_SUCCESS) {
                // TODO: Make error handling system
                throw std::runtime_error("Error: Failed to create swap chain!\n");
            }

            vkGetSwapchainImagesKHR(application->getDevice(), swapChain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(application->getDevice(), swapChain, &imageCount, swapChainImages.data());
            swapChainImageFormat = surfaceFormat.format;
            swapChainExtent = extent;
        }

        void mainLoop() 
        {
            while(!glfwWindowShouldClose(window->getGlfwWindow())) {
                glfwPollEvents();
            }
        }

        void cleanup() 
        {
            for (auto imageView : swapChainImageViews) {
                vkDestroyImageView(application->getDevice(), imageView, nullptr);
            }
            vkDestroySwapchainKHR(application->getDevice(), this->swapChain, nullptr);
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
