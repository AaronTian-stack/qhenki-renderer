#include "framebufferattachment.h"

FrameBufferAttachment::FrameBufferAttachment(
        vk::Image image, vk::ImageView imageView, vk::Format format)
: allocator(nullptr), allocation(nullptr), image(image), imageView(imageView), format(format)
{}

FrameBufferAttachment::FrameBufferAttachment(
        vk::Device device, VmaAllocator allocator, VmaAllocation allocation,
        vk::Image image, vk::ImageView imageView, vk::Format format)
: Destroyable(device), allocator(allocator), allocation(allocation), image(image), imageView(imageView), format(format)
{}

void FrameBufferAttachment::destroy()
{
    device.destroyImageView(imageView);
    vmaDestroyImage(allocator, image, allocation);
    if (sampler != nullptr)
        device.destroySampler(sampler);
}

vk::DescriptorImageInfo FrameBufferAttachment::getDescriptorInfo()
{
    return {sampler, imageView, vk::ImageLayout::eShaderReadOnlyOptimal};
}

void FrameBufferAttachment::createGenericSampler()
{
    vk::SamplerCreateInfo samplerInfo{
            vk::SamplerCreateFlags(),
            vk::Filter::eLinear,
            vk::Filter::eLinear,
            vk::SamplerMipmapMode::eLinear,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            0.0f,
            VK_FALSE, // TODO: needs hardware support
            16,
            VK_FALSE,
            vk::CompareOp::eAlways,
            0.0f,
            0.0f,
            vk::BorderColor::eIntOpaqueBlack,
            VK_FALSE
    };
    sampler = device.createSampler(samplerInfo);
}
