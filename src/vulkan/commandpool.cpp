#include "commandpool.h"

void CommandPool::create(VkDevice device, QueueFamilyIndices queueFamilyIndices)
{
    deviceForDispose = device;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void CommandPool::dispose()
{
    vkDestroyCommandPool(deviceForDispose, commandPool, nullptr);
}

VkCommandBuffer CommandPool::createCommandBuffer()
{
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // TODO: abstract so you can create secondary command buffers
    allocInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(deviceForDispose, &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    return commandBuffer;
}

VkCommandBuffer CommandPool::createCommandBuffer(const char* name)
{
    VkCommandBuffer commandBuffer = createCommandBuffer();
    commandBuffers[name] = commandBuffer;
    return commandBuffer;
}

VkCommandBuffer CommandPool::getCommandBuffer(const char *name)
{
    return commandBuffers[name];
}

VkCommandBuffer CommandPool::beginSingleCommand()
{
    auto commandBuffer = createCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void CommandPool::endSingleTimeCommands(QueueManager &queueManager, VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queueManager.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queueManager.graphicsQueue);

    vkFreeCommandBuffers(deviceForDispose, commandPool, 1, &commandBuffer);
}
