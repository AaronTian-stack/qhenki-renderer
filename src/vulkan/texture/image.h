#pragma once

#include "vulkan/vulkan.hpp"
#include "../queuemanager.h"
#include "../commandpool.h"

class Image : public Destroyable
{
private:
    sPtr<FrameBufferAttachment> attachment;
    bool destroyed;

public:
    Image(const sPtr<FrameBufferAttachment> attachment);

    static void recordTransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                            vk::Image image, vk::CommandBuffer commandBuffer);
    void recordTransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                     vk::CommandBuffer commandBuffer);
    void destroy() override;

    friend class Texture;
    friend class BufferFactory;
};
