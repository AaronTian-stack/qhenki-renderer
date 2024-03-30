#pragma once

#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "../context/vulkancontext.h"
#include "buffer.h"
#include "../../smartpointer.h"
#include "../texture/image.h"
#include "../texture/texture.h"

struct ImageAndImageView
{
    vk::Image image;
    vk::ImageView imageView;
    VmaAllocation allocation;
};

class BufferFactory : public Destroyable
{
private:
    VmaAllocator allocator;
    static vk::ImageCreateInfo imageInfo(vk::Format format, vk::Extent3D extent, vk::ImageUsageFlags usage);
    static vk::ImageViewCreateInfo imageViewInfo(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);

public:
    void create(VulkanContext &context);

    uPtr<Buffer> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaAllocationCreateFlagBits flags = static_cast<VmaAllocationCreateFlagBits>(0));
    uPtr<FrameBufferAttachment> createAttachment(vk::Format format, vk::Extent3D extent,
                                                 vk::ImageUsageFlags imageUsage, vk::ImageAspectFlags aspectFlags);

    uPtr<Image> createTextureImage(CommandPool &commandPool, QueueManager &queueManager,
                                   vk::Format format, vk::Extent3D extent,
                                   vk::Flags<vk::ImageUsageFlagBits> imageUsage, vk::Flags<vk::ImageAspectFlagBits> aspectFlags,
                                   void *data);

    void destroy();
};
