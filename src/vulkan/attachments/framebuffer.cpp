#include "framebuffer.h"

FrameBuffer::FrameBuffer(vk::Device device, vk::Framebuffer framebuffer)
        : Destroyable(device), framebuffer(framebuffer)
{}

FrameBuffer::FrameBuffer(vk::Device device, vk::Framebuffer framebuffer, std::vector<uPtr<Attachment>> &attachments)
        : FrameBuffer(device, framebuffer)
{
    for (auto &attachment : attachments)
    {
        attachment->create(device);
        this->attachments.push_back(std::move(attachment));
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

std::vector<vk::DescriptorImageInfo> FrameBuffer::getDescriptorInfo(const std::vector<vk::Sampler> &samplers)
{
    std::vector<vk::DescriptorImageInfo> descriptorImageInfo;
    descriptorImageInfo.reserve(attachments.size());

    for (int i = 0; i < attachments.size(); i++)
    {
        descriptorImageInfo.push_back(attachments[i]->getDescriptorInfo(samplers[std::min(i, (int)samplers.size() - 1)]));
    }
    return descriptorImageInfo;
}
