#include "framebuffer.h"

FrameBuffer::FrameBuffer(vk::Device device, vk::Framebuffer framebuffer, std::vector<FrameBufferAttachment> attachments)
        : Destroyable(device), framebuffer(framebuffer), attachments(attachments)
{
    for (auto &attachment : attachments)
    {
        attachment.create(device);
    }
}

void FrameBuffer::destroy()
{
    device.destroy(framebuffer);
    for (auto &attachment : attachments)
    {
        attachment.destroy();
    }
}
