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
    bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive; // TODO: ownership transfer

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

vk::ImageCreateInfo BufferFactory::imageInfo(vk::Format format, vk::Extent3D extent, vk::ImageUsageFlags usage)
{
    // TODO: mip levels
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
            vk::SharingMode::eExclusive,
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

sPtr<Attachment> BufferFactory::createAttachment(vk::ImageCreateInfo imageCreateInfo, vk::ImageViewCreateInfo imageViewCreateInfo, vk::Format format)
{
    vk::Image image;
    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    VmaAllocation allocation;
    vmaCreateImage(allocator, (VkImageCreateInfo*) &imageCreateInfo, &allocationCreateInfo, (VkImage*) &image, &allocation, nullptr);

    imageViewCreateInfo.image = image;
    vk::ImageView imageView;
    auto result = device.createImageView(&imageViewCreateInfo, nullptr, &imageView);
    if (result != vk::Result::eSuccess)
        throw std::runtime_error("failed to create image view");

    return mkS<Attachment>(device, allocator, allocation, image, imageView, format);
}

sPtr<Attachment> BufferFactory::createAttachment(
    vk::Format format, vk::Extent3D extent,
    vk::ImageUsageFlags imageUsage, vk::ImageAspectFlags aspectFlags)
{
    // extent: resolution ie 1280,720,1
    // usage: ex eDepthStencil
    // aspectFlags: ex eDepth

    vk::Image image;
    auto imageCreateInfo = imageInfo(format, extent, imageUsage);

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    VmaAllocation allocation;
    vmaCreateImage(allocator, (VkImageCreateInfo*) &imageCreateInfo, &allocationCreateInfo, (VkImage*) &image, &allocation, nullptr);

    vk::ImageView imageView;
    auto imageViewCreateInfo = imageViewInfo(image, format, aspectFlags);
    auto result = device.createImageView(&imageViewCreateInfo, nullptr, &imageView);
    if (result != vk::Result::eSuccess)
        throw std::runtime_error("failed to create image view");

    return mkS<Attachment>(device, allocator, allocation, image, imageView, format);
}

DeferredImage BufferFactory::createTextureImageDeferred(
        CommandPool &commandPool,
        vk::Format format, vk::Extent3D extent, vk::Flags<vk::ImageUsageFlagBits> imageUsage,
        vk::Flags<vk::ImageAspectFlagBits> aspectFlags, void *data)
{
    auto texture = mkU<Image>(createAttachment(format, extent, imageUsage, aspectFlags));
    texture->attachment->create(this->device);

    // copy data to buffer
    auto stagingBuffer = createBuffer(
            extent.width * extent.height * extent.depth * 4, // TODO: check this
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    stagingBuffer->fill(data);

    auto commandBuffer = commandPool.beginSingleCommand();

    texture->recordTransitionImageLayout(
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal,
            commandBuffer);

    // vk cmd copy to buffer
    vk::BufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = extent;

    commandBuffer.copyBufferToImage(stagingBuffer->buffer, texture->attachment->image,
                                    vk::ImageLayout::eTransferDstOptimal, 1, &region);

    texture->recordTransitionImageLayout(
            vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            commandBuffer);

    commandBuffer.end();

    return {std::move(texture), commandBuffer, std::move(stagingBuffer)};
}

uPtr<Image> BufferFactory::createTextureImage(CommandPool &commandPool, QueueManager &queueManager,
                                              vk::Format format, vk::Extent3D extent, vk::Flags<vk::ImageUsageFlagBits> imageUsage,
                                              vk::Flags<vk::ImageAspectFlagBits> aspectFlags, void *data)
{
    auto deferredImage = createTextureImageDeferred(commandPool, format, extent, imageUsage, aspectFlags, data);
    auto texture = std::move(deferredImage.image);
    auto commandBuffer = deferredImage.cmd;
    auto stagingBuffer = std::move(deferredImage.stagingBufferToDestroy);

    commandPool.submitSingleTimeCommands(queueManager, {commandBuffer}, true);

    stagingBuffer->destroy();

    return texture;
}
