#include "commandpool.h"

void CommandPool::create(Device &device)
{
    this->device = vk::Device(device.vkbDevice.device);
    auto poolInfo = vk::CommandPoolCreateInfo(
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            device.vkbDevice.get_queue_index(vkb::QueueType::graphics).value()
            );
    commandPool = this->device.createCommandPool(poolInfo);
    singleCommandFence = this->device.createFence(vk::FenceCreateInfo());
}

void CommandPool::destroy()
{
    device.destroyFence(singleCommandFence);
    device.destroy(commandPool);
}

vk::CommandBuffer CommandPool::createCommandBuffer(vk::CommandBufferLevel level)
{
    auto allocInfo = vk::CommandBufferAllocateInfo(
            commandPool,
            level,
            1
            );
    return device.allocateCommandBuffers(allocInfo)[0];
}

vk::CommandBuffer CommandPool::createCommandBuffer(const char* name, vk::CommandBufferLevel level)
{
    auto commandBuffer = createCommandBuffer(level);
    commandBuffers[name] = commandBuffer;
    return commandBuffer;
}

vk::CommandBuffer CommandPool::getCommandBuffer(const char *name)
{
    return commandBuffers[name];
}

vk::CommandBuffer CommandPool::beginSingleCommand()
{
    auto commandBuffer = createCommandBuffer();

    auto beginInfo = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void CommandPool::submitSingleTimeCommands(QueueManager &queueManager, std::vector<vk::CommandBuffer> commandBuffers)
{
    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    auto result = queueManager.graphicsQueue.submit(1, &submitInfo, singleCommandFence);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to submit single time command buffer!");
    }
    auto wait = device.waitForFences(singleCommandFence, VK_TRUE, UINT64_MAX);
    if (wait != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to wait for fence!");
    }

    device.freeCommandBuffers(commandPool, commandBuffers.size(), commandBuffers.data());
    device.resetFences(singleCommandFence);
}
