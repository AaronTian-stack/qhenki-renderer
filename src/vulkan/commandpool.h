#pragma once

#include <vulkan/vulkan.h>
#include "../disposable.h"
#include "devicepicker.h"

class CommandPool : public Disposable
{
private:
    VkCommandPool commandPool;
    std::unordered_map<const char*, VkCommandBuffer> commandBuffers;

public:
    void create(VkDevice device, QueueFamilyIndices queueFamilyIndices);
    void dispose() override;

    VkCommandBuffer createCommandBuffer();
    VkCommandBuffer createCommandBuffer(const char* name);
    VkCommandBuffer getCommandBuffer(const char* name);
};
