#include "bufferfactory.h"

void BufferFactory::create(VulkanContext &context)
{
    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorCreateInfo.physicalDevice = context.device.physicalDevice;
    allocatorCreateInfo.device = context.device.logicalDevice;
    allocatorCreateInfo.instance = context.vkbInstance.instance;
    allocatorCreateInfo.pVulkanFunctions = nullptr;

    if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS)
        throw std::runtime_error("failed to create allocator");

    Destroyable::create(context.device.logicalDevice);
}

uPtr<Buffer> BufferFactory::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaAllocationCreateFlagBits flags)
{
    vk::BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = vk::StructureType::eBufferCreateInfo;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.flags = flags; // VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT

    VkBuffer buffer;
    VmaAllocation allocation;
    if (vmaCreateBuffer(allocator, (VkBufferCreateInfo *)(&bufferCreateInfo), &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer");

    return mkU<Buffer>(vk::Buffer(buffer), vk::BufferCreateInfo(bufferCreateInfo), allocation, allocator, flags & VMA_ALLOCATION_CREATE_MAPPED_BIT);
}

void BufferFactory::destroy()
{
    vmaDestroyAllocator(allocator);
}

vk::ImageCreateInfo BufferFactory::imageInfo(vk::Format format, vk::Extent3D extent, vk::ImageUsageFlagBits usage)
{
    vk::ImageCreateInfo info = {
            vk::ImageCreateFlags(),
            vk::ImageType::e2D,
            format,
            extent,
            1,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            usage,
    };
    return info;
}

vk::ImageViewCreateInfo BufferFactory::imageViewInfo(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageViewCreateInfo info;
    info.sType = vk::StructureType::eImageViewCreateInfo;
    info.pNext = nullptr;

    info.viewType = vk::ImageViewType::e2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspectFlags;
    return info;
}

FrameBufferAttachment BufferFactory::createAttachment(
    vk::Format format, vk::Extent3D extent,
    vk::ImageUsageFlagBits imageUsage, vk::ImageAspectFlagBits aspectFlags)
{
    /*vk::Extent3D depthImageExtent = {
            1280,
            720,
            1
    };
    vk::Format format = vk::Format::eD32Sfloat;
     vk::ImageUsageFlagBits imageUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment
     vk::ImageAspectFlagBits aspectFlags = vk::ImageAspectFlagBits::eDepth;
     */

    vk::Image image;
    auto imageCreateInfo = imageInfo(format, extent, imageUsage);

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    VmaAllocation allocation;
    vmaCreateImage(allocator, (VkImageCreateInfo *) &imageCreateInfo, &allocationCreateInfo, (VkImage *) &image, &allocation, nullptr);

    vk::ImageView imageView;
    auto imageViewCreateInfo = imageViewInfo(image, format, aspectFlags);
    auto result = device.createImageView(&imageViewCreateInfo, nullptr, &imageView);
    if (result != vk::Result::eSuccess)
        throw std::runtime_error("failed to create image view");

    return {allocator, allocation, image, imageView, format};
}

