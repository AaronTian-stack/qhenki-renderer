#pragma once

#include "vulkan/vulkan.hpp"
#include "../queuemanager.h"
#include "../commandpool.h"

class Texture : public Destroyable
{
private:
    uPtr<FrameBufferAttachment> attachment;
    // TODO: sampler

public:
    Texture(uPtr<FrameBufferAttachment> attachment);
    void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout, CommandPool &commandPool, QueueManager &queueManager);

    void destroy() override;

    friend class BufferFactory;
};
