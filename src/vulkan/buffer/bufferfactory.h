
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "../context/vulkancontext.h"
#include "buffer.h"
#include <smartpointer.h>
#include "../texture/image.h"
#include "../texture/texture.h"

struct ImageAndImageView
{
    vk::Image image;
    vk::ImageView imageView;
    VmaAllocation allocation;
};

struct DeferredImage
{
    uPtr<Image> image;
    vk::CommandBuffer cmd;
    uPtr<Buffer> stagingBufferToDestroy;
};

class BufferFactory : public Destroyable
{
private:
    VmaAllocator allocator;
    static vk::ImageCreateInfo imageInfo(vk::Format format, vk::Extent3D extent, vk::ImageUsageFlags usage);
    static vk::ImageViewCreateInfo imageViewInfo(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);

public:
    void create(VulkanContext &context);

    uPtr<Buffer> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaAllocationCreateFlags flags);
    uPtr<Attachment> createAttachment(vk::ImageCreateInfo imageCreateInfo, vk::ImageViewCreateInfo imageViewCreateInfo, vk::Format format);
    uPtr<Attachment> createAttachment(vk::Format format, vk::Extent3D extent,
                                      vk::ImageUsageFlags imageUsage, vk::ImageAspectFlags aspectFlags);

    void destroy();

    DeferredImage createTextureImageDeferred(CommandPool &commandPool, vk::Format format, vk::Extent3D extent,
                               vk::Flags<vk::ImageUsageFlagBits> imageUsage,
                               vk::Flags<vk::ImageAspectFlagBits> aspectFlags,
                               void *data);

    uPtr<Image> createTextureImage(CommandPool &commandPool, QueueManager &queueManager,
                                   vk::Format format, vk::Extent3D extent,
                                   vk::Flags<vk::ImageUsageFlagBits> imageUsage, vk::Flags<vk::ImageAspectFlagBits> aspectFlags,
                                   void *data);
};
