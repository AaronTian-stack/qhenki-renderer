#pragma once

#include "vulkan/vulkan.hpp"
#include "../destroyable.h"
#include "vma/vk_mem_alloc.h"

class Attachment : public Destroyable
{
private:
    const VmaAllocator allocator;
    const VmaAllocation allocation;

public:
    const vk::Image image;
    const vk::ImageView imageView;
    const vk::Format format;
    vk::Sampler sampler = nullptr;
    Attachment(vk::Image image, vk::ImageView imageView, vk::Format format);
    Attachment(vk::Device device, VmaAllocator allocator, VmaAllocation allocation,
               vk::Image image, vk::ImageView imageView, vk::Format format);
    void createGenericSampler(vk::Filter filter, vk::SamplerMipmapMode mipmapMode);
    void createGenericSampler();
    vk::DescriptorImageInfo getDescriptorInfo();
    void destroy() override;

    friend class Image;
};