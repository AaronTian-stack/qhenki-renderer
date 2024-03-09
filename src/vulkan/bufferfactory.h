#pragma once

#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "context/vulkancontext.h"
#include "buffer.h"
#include "../smartpointer.h"

class BufferFactory : public Destroyable
{
private:
    VmaAllocator allocator;
    static vk::ImageCreateInfo imageInfo(vk::Format format, vk::Extent3D extent, vk::ImageUsageFlagBits usage);
    static vk::ImageViewCreateInfo imageViewInfo(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);

public:
    void create(VulkanContext &context);

    uPtr<Buffer> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaAllocationCreateFlagBits flags = static_cast<VmaAllocationCreateFlagBits>(0));
    uPtr<FrameBufferAttachment> createAttachment(vk::Format format, vk::Extent3D extent,
                                                 vk::ImageUsageFlagBits imageUsage, vk::ImageAspectFlagBits aspectFlags);

    void destroy();
};
