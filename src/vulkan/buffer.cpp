#include "buffer.h"
#include "bufferfactory.h"

Buffer::Buffer(vk::Buffer buffer, vk::BufferCreateInfo info, VmaAllocation allocation, VmaAllocator allocator, bool persistent)
: buffer(buffer), info(info), allocation(allocation), allocator(allocator), persistent(persistent)
{
    if (persistent)
    {
        vmaMapMemory(allocator, allocation, &mappedData);
    }
}

void Buffer::fill(const void *data)
{
    if (persistent)
    {
        memcpy(mappedData, data, info.size);
    }
    else
    {
        void* mappedData;
        vmaMapMemory(allocator, allocation, &mappedData);
        memcpy(mappedData, data, info.size);
        vmaUnmapMemory(allocator, allocation);
    }
}

void Buffer::destroy()
{
    if (persistent)
    {
        vmaUnmapMemory(allocator, allocation);
    }
    vmaDestroyBuffer(allocator, buffer, allocation);
}

void Buffer::bind(vk::CommandBuffer commandBuffer)
{
    VkDeviceSize offset = 0;
    if (info.usage & vk::BufferUsageFlagBits::eIndexBuffer)
        commandBuffer.bindIndexBuffer(buffer, offset, vk::IndexType::eUint16);
    else if (info.usage & vk::BufferUsageFlagBits::eVertexBuffer)
        commandBuffer.bindVertexBuffers(0, 1, &buffer, &offset);
}

void Buffer::copyTo(Buffer &destination, QueueManager &queueManager, CommandPool &commandPool)
{
    auto commandBuffer = commandPool.beginSingleCommand();

    vk::BufferCopy copyRegion(0, 0, this->info.size);
    commandBuffer.copyBuffer(this->buffer, destination.buffer, 1, &copyRegion);

    commandBuffer.end();
    commandPool.submitSingleTimeCommands(queueManager, {commandBuffer});
}
