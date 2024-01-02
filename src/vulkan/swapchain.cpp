#include "swapchain.h"

SwapChainSupportDetails SwapChain::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;
    // basic surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        // support surface formats
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        // support presentation modes
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0]; // if can't find, just return the first one
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        // prefer triple buffering
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    // double buffering
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(Window &window, const VkSurfaceCapabilitiesKHR &capabilities)
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

void SwapChain::createSwapChain(DevicePicker &vkDevicePicker, Window &window)
{
    deviceForDispose = vkDevicePicker.getDevice();

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(
            vkDevicePicker.getPhysicalDevice(), window.getSurface()
            );

    // surface format (color channels, color space), presentation mode (buffering), resolution
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(window, swapChainSupport.capabilities);

    // request to have +1 minimum images in swap chain
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    // do not exceed maximum image count, if it exists (0 means no maximum)
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    // what surface to create swap chain for
    createInfo.surface = window.getSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    // basically always 1 unless doing stereoscopic 3D
    createInfo.imageArrayLayers = 1;
    // what kind of operations images in swap chain will be used for
    // render directly, so use as color attachment
    // VK_IMAGE_USAGE_TRANSFER_DST_BIT if we want to render to separate image first to perform operations on it?
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // handling swap chain images used across multiple queue families
    QueueFamilyIndices indices = DevicePicker::findQueueFamilies(vkDevicePicker.getPhysicalDevice(), window.getSurface());
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        // image can be used across multiple queue families without explicit ownership transfers
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // queueFamilyIndexCount and pQueueFamilyIndices optional
    }

    // do not do any transformations to images in swap chain
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    // ignore alpha channel
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    // do not care about color of pixels that are obscured
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // create swap chain
    if (vkCreateSwapchainKHR(vkDevicePicker.getDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    // retrieve swap chain images
    vkGetSwapchainImagesKHR(vkDevicePicker.getDevice(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(vkDevicePicker.getDevice(), swapChain, &imageCount, swapChainImages.data());

    // store format and extent for later use
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    createImageViews(vkDevicePicker);
}

void SwapChain::dispose()
{
    for (auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(deviceForDispose, framebuffer, nullptr);
    }
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(deviceForDispose, imageView, nullptr);
    }
    vkDestroySwapchainKHR(deviceForDispose, swapChain, nullptr);
}

void SwapChain::createImageViews(DevicePicker &vkDevicePicker)
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        // image to create view for
        createInfo.image = swapChainImages[i];
        // how image data should be interpreted
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        // swizzling components
        createInfo.format = swapChainImageFormat;
        // color channels (default mapping)
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // subresource range describes image's purpose and which part of image should be accessed
        // color target
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // no mipmapping
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        // no 3D textures
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        // create image view
        if (vkCreateImageView(vkDevicePicker.getDevice(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

VkFormat SwapChain::getFormat() const
{
    return swapChainImageFormat;
}

void SwapChain::createFramebuffers(RenderPass &renderPass)
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] =
        {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        // render pass
        framebufferInfo.renderPass = renderPass.getRenderPass();
        // attachments
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        // width and height
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        // layers
        framebufferInfo.layers = 1;

        // create framebuffer
        if (vkCreateFramebuffer(deviceForDispose, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

VkExtent2D SwapChain::getExtent() const
{
    return swapChainExtent;
}

VkSwapchainKHR SwapChain::getSwapChain()
{
    return swapChain;
}

VkFramebuffer SwapChain::nextImage(VkSemaphore imageAvailable)
{
    vkAcquireNextImageKHR(deviceForDispose, swapChain,
                          UINT64_MAX, imageAvailable, VK_NULL_HANDLE,
                          &imageIndex);
    return swapChainFramebuffers[imageIndex];
}
