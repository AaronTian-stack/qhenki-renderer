#include "swapchain.h"

SwapChainSupportDetails SwapChain::querySwapChainSupport(const vk::PhysicalDevice &physicalDevice, VkSurfaceKHR surface)
{
    return {
    physicalDevice.getSurfaceCapabilitiesKHR(surface),
    physicalDevice.getSurfaceFormatsKHR(surface),
    physicalDevice.getSurfacePresentModesKHR(surface)
    };
}

vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        // TODO: change this back to VK_FORMAT_B8G8R8A8_SRGB
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }
    return availableFormats[0]; // if it can't find, just return the first one
}

vk::PresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eImmediate)
        {
            return availablePresentMode;
        }
    }
    // double buffering
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapChain::chooseSwapExtent(Window &window, const vk::SurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window.getWindow(), &width, &height);
        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void SwapChain::createSwapChain(Device &device, Window &window)
{
    this->device = device.logicalDevice;
    auto swapChainSupport = querySwapChainSupport(device.physicalDevice, window.getSurface());

    // surface format (color channels, color space), presentation mode (buffering), resolution
    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(window, swapChainSupport.capabilities);

    // request to have +1 minimum images in swap chain
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    // do not exceed maximum image count, if it exists (0 means no maximum)
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    auto createInfo = vk::SwapchainCreateInfoKHR(
        vk::SwapchainCreateFlagsKHR(),
        window.getSurface(),
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        extent,
        1, // always 1 unless doing stereoscopic 3D
        vk::ImageUsageFlagBits::eColorAttachment // what kind of operations images in swap chain will be used for
    );

    // handling swap chain images used across multiple queue families
    QueueFamilyIndices indices = DevicePicker::findQueueFamilies(device.physicalDevice, window.getSurface());
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        // image can be used across multiple queue families without explicit ownership transfers
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        // queueFamilyIndexCount and pQueueFamilyIndices optional
    }

    // do not do any transformations to images in swap chain
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    // ignore alpha channel
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque; //createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    // do not care about color of pixels that are obscured
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    swapChain = device.logicalDevice.createSwapchainKHR(createInfo); // create swap chain

    swapChainImages = device.logicalDevice.getSwapchainImagesKHR(swapChain); // retrieve swap chain images

    // store format and extent for later use
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    createImageViews(device.logicalDevice);
}

void SwapChain::destroy()
{
    // see vkDestroyFramebuffer
    for (auto &framebuffer : swapChainFramebuffers)
    {
        device.destroy(framebuffer);
    }
    // see vkDestroyImageView
    for (auto &imageView : swapChainImageViews)
    {
        device.destroy(imageView);
    }
    device.destroy(swapChain); // see vkDestroySwapchainKHR
}

void SwapChain::createImageViews(vk::Device &device)
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        auto createInfo = vk::ImageViewCreateInfo(
            vk::ImageViewCreateFlags(),
            swapChainImages[i], // image to create view for
            vk::ImageViewType::e2D, // how image data should be interpreted
            swapChainImageFormat,
            vk::ComponentMapping(), // color channels (default mapping)
                // subresource range describes image's purpose and which part of image should be accessed
            vk::ImageSubresourceRange(
                vk::ImageAspectFlagBits::eColor, // color target
                0, // base mip level
                1, // level count
                0, // base array layer (no 3D textures)
                1 // layer count
            )
        );
        swapChainImageViews[i] = device.createImageView(createInfo); // create image view
    }
}

vk::Format SwapChain::getFormat() const
{
    return swapChainImageFormat;
}

void SwapChain::createFramebuffers(RenderPass &renderPass)
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        vk::ImageView attachments[] =
        {
            swapChainImageViews[i]
        };

        vk::FramebufferCreateInfo createInfo(
            vk::FramebufferCreateFlags(),
            renderPass.getRenderPass(),
            1,
            attachments,
            swapChainExtent.width,
            swapChainExtent.height,
            1
        );

        swapChainFramebuffers[i] = device.createFramebuffer(createInfo);
    }
}

vk::Extent2D SwapChain::getExtent() const
{
    return swapChainExtent;
}

vk::Framebuffer SwapChain::nextImage(vk::Semaphore imageAvailable)
{
    // see vkAcquireNextImageKHR
    auto result = device.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE);
    imageIndex = result.value;
    return swapChainFramebuffers[imageIndex];
}
