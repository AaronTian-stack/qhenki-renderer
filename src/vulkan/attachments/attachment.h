#pragma once

#include <vulkan/vulkan.hpp>
#include "../destroyable.h"
#include <vma/vk_mem_alloc.h>

class Attachment : public Destroyable
{
private:
    const VmaAllocator allocator;
    const VmaAllocation allocation;

public:
    const vk::Image image;
    const vk::ImageView imageView;
    const vk::Format format;
    const vk::Extent3D extent;
    Attachment(vk::Image image, vk::ImageView imageView, vk::Format format, vk::Extent3D extent);
    Attachment(vk::Device device, VmaAllocator allocator, VmaAllocation allocation,
               vk::Image image, vk::ImageView imageView, vk::Format format, vk::Extent3D extent);
    vk::DescriptorImageInfo getDescriptorInfo(vk::Sampler sampler);
    void destroy() override;

    friend class Image;
};
