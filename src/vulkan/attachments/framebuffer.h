#pragma once

#include "attachment.h"
#include "../destroyable.h"
#include <smartpointer.h>

class FrameBuffer : public Destroyable
{
public:
    vk::Framebuffer framebuffer;
    std::vector<uPtr<Attachment>> attachments;
    FrameBuffer(vk::Device device, vk::Framebuffer framebuffer);
    FrameBuffer(vk::Device device, vk::Framebuffer framebuffer, std::vector<uPtr<Attachment>> &attachments);
    std::vector<vk::DescriptorImageInfo> getDescriptorInfo(const std::vector<vk::Sampler> &samplers);
    void destroy() override;
};
