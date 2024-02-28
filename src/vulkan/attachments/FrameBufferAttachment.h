#pragma once

#include "vulkan/vulkan.hpp"

struct FrameBufferAttachment
{
//public:
    vk::Image image;
    vk::ImageView imageView;
    vk::Format format;
    vk::DeviceMemory memory;
};
