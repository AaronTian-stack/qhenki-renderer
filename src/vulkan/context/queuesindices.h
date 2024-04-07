#pragma once

#include <vulkan/vulkan.hpp>

struct QueuesIndices
{
    vk::Queue graphics;
    vk::Queue present;
    vk::Queue transfer;

    uint32_t graphicsIndex;
    uint32_t presentIndex;
    uint32_t transferIndex;
};