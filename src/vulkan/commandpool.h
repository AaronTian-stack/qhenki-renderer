#pragma once

#include <vulkan/vulkan.hpp>
#include <unordered_map>
#include "destroyable.h"
#include "queuemanager.h"
#include "context/vulkancontext.h"

class CommandPool : public Destroyable
{
private:
    vk::Fence singleCommandFence;
    vk::CommandPool commandPool;
    std::unordered_map<const char*, vk::CommandBuffer> commandBuffers;

public:
    void create(Device &device);
    void destroy() override;

    vk::CommandBuffer createCommandBuffer();
    vk::CommandBuffer createCommandBuffer(const char* name);
    vk::CommandBuffer getCommandBuffer(const char* name);

    vk::CommandBuffer beginSingleCommand();
    void submitSingleTimeCommands(QueueManager &queueManager, std::vector<vk::CommandBuffer> commandBuffers);
};
