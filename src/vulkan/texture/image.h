#pragma once

#include "vulkan/vulkan.hpp"
#include "../queuemanager.h"
#include "../commandpool.h"

class Image : public Destroyable
{
private:
    sPtr<Attachment> attachment;
    bool destroyed;

public:
    explicit Image(const sPtr<Attachment> &attachment);

    static void recordTransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                            vk::Image image, vk::CommandBuffer commandBuffer, int mipCount, int layerCount);
    void recordTransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                     vk::CommandBuffer commandBuffer);


    static void blit(vk::Image srcImage, vk::Image dstImage, vk::CommandBuffer commandBuffer,
                    vk::ImageLayout srcLayout, vk::ImageLayout dstLayout, vk::Extent3D extent);
    void blit(Image &dstImage, vk::CommandBuffer commandBuffer);

    void destroy() override;

    friend class Texture;
    friend class BufferFactory;
    friend class EnvironmentMap;
};
