#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "devicepicker.h"
#include "renderpass.h"
#include "../window.h"
#include "device.h"

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class SwapChain : public Destroyable
{
private:
    uint32_t imageIndex; // for presentation

    //VkDevice deviceForDispose;
    vk::SwapchainKHR swapChain; // holds the images

    // VkImage is a handle to an image object, multidimensional array of data. can be used as attachments, textures, etc.
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::ImageView> swapChainImageViews; // TODO: maybe abstraction that pairs image with image view?

    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;

    // wraps the image views
    std::vector<VkFramebuffer> swapChainFramebuffers;

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

    static vk::Extent2D chooseSwapExtent(Window &window, const vk::SurfaceCapabilitiesKHR &capabilities);
    void createImageViews(vk::Device &device);

public:
    static SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice &device, VkSurfaceKHR surface);
    vk::Format getFormat() const;
    void createSwapChain(Device &device, Window &window);
    void createFramebuffers(RenderPass &renderPass);
    void destroy() override;

    vk::Extent2D getExtent() const;

    vk::Framebuffer nextImage(vk::Semaphore imageAvailable);

    friend class QueueManager;
};
