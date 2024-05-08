#pragma once

#include "attachment.h"
#include "../destroyable.h"
#include "../../smartpointer.h"

class FrameBuffer : public Destroyable
{
public:
    vk::Framebuffer framebuffer;
    std::vector<sPtr<Attachment>> attachments;
    FrameBuffer(vk::Device device, vk::Framebuffer framebuffer, const std::vector<sPtr<Attachment>> &attachments);
    std::vector<vk::DescriptorImageInfo> getDescriptorInfo();
    void destroy() override;
};
