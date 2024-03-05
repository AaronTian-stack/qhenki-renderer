#include "framebufferattachment.h"

FrameBufferAttachment::FrameBufferAttachment(
        vk::Image image, vk::ImageView imageView, vk::Format format)
: image(image), imageView(imageView), format(format)
{}

void FrameBufferAttachment::destroy()
{
    device.destroyImageView(imageView);
    vmaDestroyImage(allocator, image, allocation);
}
