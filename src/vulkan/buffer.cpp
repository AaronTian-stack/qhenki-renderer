#include "buffer.h"
#include "bufferfactory.h"

Buffer::Buffer(vk::Buffer buffer, VmaAllocation allocation, VmaAllocator allocator)
: buffer(buffer), allocation(allocation), allocator(allocator)
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
