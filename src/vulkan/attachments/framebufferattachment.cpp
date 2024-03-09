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
}
