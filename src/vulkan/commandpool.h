#pragma once

#include <vulkan/vulkan.h>
#include "../destroyable.h"
#include "devicepicker.h"
#include "queuemanager.h"

class CommandPool : public Destroyable
{
private:
    vk::CommandPool commandPool;
    std::unordered_map<const char*, vk::CommandBuffer> commandBuffers;

public:
    void create(vk::Device device, uint32_t familyIndex);
    void destroy() override;

    vk::CommandBuffer createCommandBuffer();
    vk::CommandBuffer createCommandBuffer(const char* name);
    vk::CommandBuffer getCommandBuffer(const char* name);

    vk::CommandBuffer beginSingleCommand();
    void endSingleTimeCommands(QueueManager &queueManager, vk::CommandBuffer commandBuffer);
};
