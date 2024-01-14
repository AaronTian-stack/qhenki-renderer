#include "buffer.h"
#include "bufferfactory.h"

Buffer::Buffer(vk::Buffer buffer, vk::BufferCreateInfo info, VmaAllocation allocation, VmaAllocator allocator)
: buffer(buffer), info(info), allocation(allocation), allocator(allocator)
{}

void Buffer::fill(const void *data, size_t bufferSize)
{
    void* mappedData;
    vmaMapMemory(allocator, allocation, &mappedData);
    memcpy(mappedData, data, bufferSize);
    vmaUnmapMemory(allocator, allocation);
}

void Buffer::destroy()
{
    vmaDestroyBuffer(allocator, buffer, allocation);
}

void Buffer::bind(vk::CommandBuffer commandBuffer)
{
    VkDeviceSize offset = 0;
    commandBuffer.bindVertexBuffers(0, 1, &buffer, &offset);
}

void Buffer::copyTo(Buffer &destination, QueueManager &queueManager, CommandPool &commandPool)
{
    /*VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);*/
    auto commandBuffer = commandPool.beginSingleCommand();

    /*VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);*/

    vk::BufferCopy copyRegion(0, 0, this->info.size);
    commandBuffer.copyBuffer(this->buffer, destination.buffer, 1, &copyRegion);

    //vkEndCommandBuffer(commandBuffer);

    /*VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);*/

    commandPool.endSingleTimeCommands(queueManager, {commandBuffer});
}

Buffer::~Buffer()
{
    destroy();
}

