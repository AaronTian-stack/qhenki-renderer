#pragma once

#include <vulkan/vulkan.h>
#include <unordered_map>
#include "../destroyable.h"
#include "devicepicker.h"
#include "queuemanager.h"

class CommandPool : public Destroyable
{
private:
    vk::Fence singleCommandFence;
    vk::CommandPool commandPool;
    std::unordered_map<const char*, vk::CommandBuffer> commandBuffers;

public:
    void create(vk::Device device, uint32_t familyIndex);
    void destroy() override;

    vk::CommandBuffer createCommandBuffer();
    vk::CommandBuffer createCommandBuffer(const char* name);
    vk::CommandBuffer getCommandBuffer(const char* name);

    vk::CommandBuffer beginSingleCommand();
    void submitSingleTimeCommands(QueueManager &queueManager, std::vector<vk::CommandBuffer> commandBuffers);
};
