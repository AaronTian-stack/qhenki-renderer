#pragma once

#include <vulkan/vulkan.hpp>
#include <unordered_map>
#include "destroyable.h"
#include "queuemanager.h"
#include "context/vulkancontext.h"

class CommandPool : public Destroyable
{
private:
    vk::Fence blockCommandFence;
    std::vector<vk::Fence> waitingFences;
    vk::CommandPool commandPool;
    std::unordered_map<const char*, vk::CommandBuffer> commandBuffers;
    vkb::QueueType queueType;

public:
    CommandPool(Device &device, vkb::QueueType queueType, uint32_t queueFamilyIndex);

    void destroy() override;

    vk::CommandBuffer createCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
    vk::CommandBuffer createCommandBuffer(const char* name, vk::CommandBufferLevel level);
    vk::CommandBuffer getCommandBuffer(const char* name);

    vk::CommandBuffer beginSingleCommand();
    void submitSingleTimeCommands(QueueManager &queueManager, std::vector<vk::CommandBuffer> commandBuffers, bool wait);

    friend class GLTFLoader;
};
