#pragma once

#include "FrameBufferAttachment.h"

struct FrameBuffer
{
//public:
    vk::Framebuffer framebuffer;
    std::vector<FrameBufferAttachment> attachments;
};
