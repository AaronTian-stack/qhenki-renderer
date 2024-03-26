#pragma once

#include "vulkan/vulkan.hpp"
#include "../queuemanager.h"
#include "../commandpool.h"

class Texture : public Destroyable
{
private:
    uPtr<FrameBufferAttachment> attachment;
    vk::Sampler sampler = nullptr;
    void createSampler();

public:
    Texture(uPtr<FrameBufferAttachment> attachment);

    void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout, CommandPool &commandPool, QueueManager &queueManager);

    vk::DescriptorImageInfo getDescriptorInfo();
    void destroy() override;

    friend class BufferFactory;
};
