#pragma once

#include "vulkan/vulkan.hpp"
#include "../destroyable.h"
#include "vma/vk_mem_alloc.h"

class FrameBufferAttachment : public Destroyable
{
private:
    VmaAllocator allocator;
    VmaAllocation allocation;

public:
    const vk::Image image;
    const vk::ImageView imageView;
    const vk::Format format;
    FrameBufferAttachment(vk::Image image, vk::ImageView imageView, vk::Format format);
    void destroy() override;
};
