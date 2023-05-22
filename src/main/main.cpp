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
        const uint32_t m_WIDTH  = 800;
        const uint32_t m_HEIGHT = 600;

        // TODO: Put these in another file
        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;
        VkRenderPass m_renderPass;
        std::vector<VkFramebuffer> swapChainFramebuffers;

        // Command Pool
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer; // Allocates command buffers.

        /**
         * We'll need one semaphore to signal that an image has been acquired from the swapchain and is ready for rendering, 
         * another one to signal that rendering has finished and presentation can happen, and a fence to make sure only one frame is rendering at a time.
         */
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence inFlightFence;

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
            createRenderPass();
            application->createGraphicsPipeline(this->m_renderPass);
            createFramebuffers();
            createCommandPool();
            createCommandBuffer();
            createSyncObjects();
        }

        void createSyncObjects()
        {
            // Config Semaphore.
            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            // Config Fence.
            VkFenceCreateInfo fenceInfo{};
            // fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            // Create Semaphore and Fence.
            if (vkCreateSemaphore(this->application->getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
                vkCreateSemaphore(this->application->getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
                vkCreateFence(this->application->getDevice(), &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
                throw std::runtime_error("failed to create semaphores!");
            }
        }

        void createFramebuffers()
        {
            // Resize the container to hold all of the framebuffers
            swapChainFramebuffers.resize(swapChainImageViews.size());

            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                VkImageView attachments[] = {
                    swapChainImageViews[i]
                };

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = this->m_renderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = attachments;
                framebufferInfo.width = swapChainExtent.width;
                framebufferInfo.height = swapChainExtent.height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(application->getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create framebuffer!");
                }
            }
        }

        void createCommandPool()
        {
            QueueFamilyIndices queueFamilyIndices = findQueueFamilies(this->application->getPhysicalDevice(), this->application->getVkSurface());

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

            if (vkCreateCommandPool(this->application->getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
               throw std::runtime_error("Error: Command Pool creation has failed.");
            }
        }

        void createCommandBuffer()
        {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool; 
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            // Allocate command buffers
            if (vkAllocateCommandBuffers(application->getDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
                throw std::runtime_error("Error: Failed to allocate Command buffers.");
            }
        }

        /**
         * @brief Function that writes the commands we want to execute into a command buffer.
         * 
         * @param commandBuffer 
         * @param imageIndex 
         */
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0;
            beginInfo.pInheritanceInfo = nullptr;

            if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = this->m_renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapChainExtent;

            VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            // Bind Graphics Pipeline
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->application->getGraphicsPipeline());

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChainExtent.width);
            viewport.height = static_cast<float>(swapChainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
            vkCmdEndRenderPass(commandBuffer);

            // Finished recording the command buffer.
            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                throw std::runtime_error("Error: Failed to record Command Buffer.");
            }
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

        void createRenderPass()
        {
        	VkAttachmentDescription colorAttachment{};
        	colorAttachment.format = swapChainImageFormat;
        	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        
        	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        
        	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        
        	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Specifies which layout the image will have before the render pass begins.
        	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Specifies the layout to automatically transition to when the render pass finishes.
            
            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0; // Specifies which attachment to reference by its index in the attachment descriptions array.
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Specifies which layout we would like the attachment to have during a subpass that uses this reference.
            
            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            subpass.colorAttachmentCount = 1; // The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive.
            subpass.pColorAttachments = &colorAttachmentRef;

            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;

            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;

            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments    = &colorAttachment;
            renderPassInfo.subpassCount    = 1;
            renderPassInfo.pSubpasses      = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;
            
            if (vkCreateRenderPass(application->getDevice(), &renderPassInfo, nullptr, &(this->m_renderPass)) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create the render pass.\n");
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
                drawFrame();
            }
            vkDeviceWaitIdle(this->application->getDevice());
        }

        void drawFrame()
        {
            // Wait until the previous frame has finished.
            vkWaitForFences(this->application->getDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
            
            // Manually reset the fence to the unsignaled state.
            vkResetFences(this->application->getDevice(), 1, &inFlightFence);

            // Acquire an image from the swap chain.
            uint32_t imageIndex;
            vkAcquireNextImageKHR(application->getDevice(), this->swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

            vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
            recordCommandBuffer(commandBuffer, imageIndex); // Record commands we want

            // Queue submission and synchronization.
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            if (vkQueueSubmit(this->application->getGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
               throw std::runtime_error("Error: Failed to submit draw command buffer.");
            }

            // Submit the result back to the swap chain.
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapChains[] = {swapChain};
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &imageIndex;

            presentInfo.pResults = nullptr;

            // Submit the request to present an image to the swap chain. 
            vkQueuePresentKHR(this->application->getPresentQueue(), &presentInfo);
        }

        void cleanup() 
        {
            vkDestroySemaphore(this->application->getDevice(), imageAvailableSemaphore, nullptr);
            vkDestroySemaphore(this->application->getDevice(), renderFinishedSemaphore, nullptr);
            vkDestroyFence(this->application->getDevice(), inFlightFence, nullptr);

            vkDestroyCommandPool(this->application->getDevice(), commandPool, nullptr);

            for (auto framebuffer : swapChainFramebuffers) {
                vkDestroyFramebuffer(application->getDevice(), framebuffer, nullptr);
            }

            vkDestroyPipeline(application->getDevice(), application->getGraphicsPipeline(), nullptr);
            vkDestroyPipelineLayout(application->getDevice(), application->getPipelineLayout(), nullptr);
            vkDestroyRenderPass(application->getDevice(), this->m_renderPass, nullptr);
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
