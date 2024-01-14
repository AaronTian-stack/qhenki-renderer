#include "commandpool.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-result"
void CommandPool::create(vk::Device device, uint32_t familyIndex)
{
    this->device = device;
    auto poolInfo = vk::CommandPoolCreateInfo(
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            familyIndex
            );
    commandPool = device.createCommandPool(poolInfo);
    singleCommandFence = device.createFence(vk::FenceCreateInfo());
}

void CommandPool::destroy()
{
    device.destroyFence(singleCommandFence);
    device.destroy(commandPool);
}

vk::CommandBuffer CommandPool::createCommandBuffer()
{
    auto allocInfo = vk::CommandBufferAllocateInfo(
            commandPool,
            vk::CommandBufferLevel::ePrimary, // TODO: abstract so you can create secondary command buffers
            1
            );
    return device.allocateCommandBuffers(allocInfo)[0];
}

vk::CommandBuffer CommandPool::createCommandBuffer(const char* name)
{
    auto commandBuffer = createCommandBuffer();
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

void CommandPool::endSingleTimeCommands(QueueManager &queueManager, std::vector<vk::CommandBuffer> commandBuffers)
{
    for (auto commandBuffer : commandBuffers)
    {
        commandBuffer.end();
    }

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    queueManager.graphicsQueue.submit(1, &submitInfo, singleCommandFence);
    device.waitForFences(singleCommandFence, VK_TRUE, UINT64_MAX);

    device.freeCommandBuffers(commandPool, commandBuffers.size(), commandBuffers.data());
    device.resetFences(singleCommandFence);
}

#pragma clang diagnostic pop