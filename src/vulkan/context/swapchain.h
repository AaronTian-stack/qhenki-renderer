#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "../renderpass.h"
#include "VkBootstrap.h"
#include "../attachments/framebuffer.h"

class SwapChain
{
private:
    vkb::Swapchain vkbSwapchain;
    vk::SwapchainKHR swapChain;
    uint32_t imageIndex;
    std::vector<FrameBuffer> frameBuffers;

public:
    SwapChain(vkb::Swapchain vkbSwapchain);
    void createFramebuffers(vk::RenderPass renderPass);
    vk::Format getFormat() const;
    vk::Extent2D getExtent() const;
    vk::Framebuffer nextImage(vk::Semaphore imageAvailable);
    void destroy();

    friend class VulkanContext;
    friend class QueueManager;
};
