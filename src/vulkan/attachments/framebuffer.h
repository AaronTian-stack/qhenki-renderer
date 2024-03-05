#pragma once

#include "framebufferattachment.h"
#include "../destroyable.h"

class FrameBuffer : public Destroyable
{
public:
    vk::Framebuffer framebuffer;
    std::vector<FrameBufferAttachment> attachments;
    FrameBuffer(vk::Device device, vk::Framebuffer framebuffer, std::vector<FrameBufferAttachment> attachments);
    void destroy() override;
};
