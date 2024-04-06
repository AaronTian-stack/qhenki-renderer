#pragma once

#include "framebufferattachment.h"
#include "../destroyable.h"
#include "../../smartpointer.h"

class FrameBuffer : public Destroyable
{
public:
    vk::Framebuffer framebuffer;
    std::vector<sPtr<FrameBufferAttachment>> attachments;
    FrameBuffer(vk::Device device, vk::Framebuffer framebuffer, const std::vector<sPtr<FrameBufferAttachment>>& attachments);
    std::vector<vk::DescriptorImageInfo> getDescriptorInfo();
    void destroy() override;
};
