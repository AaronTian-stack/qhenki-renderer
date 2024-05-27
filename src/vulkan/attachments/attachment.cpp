#include "attachment.h"

Attachment::Attachment(
        vk::Image image, vk::ImageView imageView, vk::Format format, vk::Extent3D extent)
: allocator(nullptr), allocation(nullptr), image(image), imageView(imageView), format(format), extent(extent)
{}

Attachment::Attachment(
        vk::Device device, VmaAllocator allocator, VmaAllocation allocation,
        vk::Image image, vk::ImageView imageView, vk::Format format, vk::Extent3D extent)
: Destroyable(device), allocator(allocator), allocation(allocation), image(image), imageView(imageView), format(format)
, extent(extent)
{}

void Attachment::destroy()
{
    device.destroyImageView(imageView);
    vmaDestroyImage(allocator, image, allocation);
    if (sampler != nullptr)
        device.destroySampler(sampler);
}

vk::DescriptorImageInfo Attachment::getDescriptorInfo()
{
    return {sampler, imageView, vk::ImageLayout::eShaderReadOnlyOptimal};
}

void Attachment::createGenericSampler(vk::Filter filter, vk::SamplerMipmapMode mipmapMode)
{
    vk::SamplerCreateInfo samplerInfo{
            vk::SamplerCreateFlags(),
            filter,
            filter,
            mipmapMode,
            vk::SamplerAddressMode::eClampToEdge,
            vk::SamplerAddressMode::eClampToEdge,
            vk::SamplerAddressMode::eClampToEdge,
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

void Attachment::createGenericSampler()
{
    createGenericSampler(vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);
}