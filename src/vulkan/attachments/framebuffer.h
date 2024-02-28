#pragma once

#include "framebufferattachment.h"

struct FrameBuffer
{
//public:
    vk::Framebuffer framebuffer;
    std::vector<FrameBufferAttachment> attachments;
};
