#include "framebuffer.h"

FrameBuffer::FrameBuffer(vk::Device device, vk::Framebuffer framebuffer, const std::vector<sPtr<Attachment>>& attachments)
        : Destroyable(device), framebuffer(framebuffer), attachments(attachments)
{
    for (auto &attachment : attachments)
    {
        attachment->create(device);
    }
}

void FrameBuffer::destroy()
{
    device.destroy(framebuffer);
    for (auto &attachment : attachments)
    {
        attachment->destroy();
    }
}

std::vector<vk::DescriptorImageInfo> FrameBuffer::getDescriptorInfo()
{
    std::vector<vk::DescriptorImageInfo> descriptorImageInfo;
    descriptorImageInfo.reserve(attachments.size());
    for (auto &attachment : attachments)
    {
        descriptorImageInfo.push_back(attachment->getDescriptorInfo());
    }
    return descriptorImageInfo;
}
