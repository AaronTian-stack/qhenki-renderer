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
}

vk::DescriptorImageInfo Attachment::getDescriptorInfo(vk::Sampler sampler)
{
    return {sampler, imageView, vk::ImageLayout::eShaderReadOnlyOptimal};
}