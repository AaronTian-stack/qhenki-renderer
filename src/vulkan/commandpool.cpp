#include "commandpool.h"

CommandPool::CommandPool(Device &device, vkb::QueueType queueType, uint32_t queueFamilyIndex)
{
    this->device = vk::Device(device.vkbDevice.device);
    this->queueType = queueType;
    auto poolInfo = vk::CommandPoolCreateInfo(
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            queueFamilyIndex
            );
    commandPool = this->device.createCommandPool(poolInfo);
    blockCommandFence = this->device.createFence(vk::FenceCreateInfo());
}

void CommandPool::destroy()
{
    device.destroyFence(blockCommandFence);
    device.destroy(commandPool);
    for (auto fence : waitingFences)
    {
        device.destroy(fence);
    }
}

vk::CommandBuffer CommandPool::createCommandBuffer(vk::CommandBufferLevel level)
{
    auto allocInfo = vk::CommandBufferAllocateInfo(commandPool,level,1);
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

void CommandPool::submitSingleTimeCommands(QueueManager &queueManager, std::vector<vk::CommandBuffer> commandBuffers, bool wait)
{
    if (!wait) throw std::runtime_error("async not implemented yet");

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    vk::Queue queueToUse;
    switch(this->queueType) {
        case vkb::QueueType::graphics:
            queueToUse = queueManager.queuesIndices.graphics;
            break;
        case vkb::QueueType::present:
            queueToUse = queueManager.queuesIndices.present;
            break;
        case vkb::QueueType::transfer:
            queueToUse = queueManager.queuesIndices.transfer;
            break;
        default:
            queueToUse = queueManager.queuesIndices.graphics;
    }
    vk::Fence fence;
    if (wait)
    {
        fence = blockCommandFence;
    }
    else
    {
        fence = device.createFence(vk::FenceCreateInfo());
        waitingFences.push_back(fence);
    }

    auto result = queueToUse.submit(1, &submitInfo, fence);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to submit single time command buffer!");
    }

    // TODO: don't wait on this when doing async!!
    if (wait)
    {
        // wait for singular fence
        auto waitResult = device.waitForFences(fence, VK_TRUE, UINT64_MAX);
        if (waitResult != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to wait for fence!");
        }

        device.freeCommandBuffers(commandPool, commandBuffers.size(), commandBuffers.data());
        device.resetFences(fence);
    }
}
