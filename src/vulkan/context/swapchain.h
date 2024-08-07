#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "../renderpass/renderpass.h"
#include <vk-bootstrap/VkBootstrap.h>
#include "../attachments/framebuffer.h"

class SwapChain
{
private:
    vkb::Swapchain vkbSwapchain;
    vk::SwapchainKHR swapChain;
    uint32_t imageIndex;
    std::vector<uPtr<FrameBuffer>> frameBuffers;

public:
    SwapChain(vkb::Swapchain vkbSwapchain);
    void createFramebuffers(vk::RenderPass renderPass, vk::ImageView depthImageView = nullptr);
    vk::Format getFormat() const;
    vk::Extent2D getExtent() const;
    FrameBuffer* nextImage(vk::Semaphore imageAvailable);
    void destroy();

    friend class VulkanContext;
    friend class QueueManager;
};
