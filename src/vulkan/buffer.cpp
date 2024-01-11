#include "buffer.h"
#include "bufferfactory.h"

Buffer::Buffer(VkBuffer buffer, VmaAllocation allocation, VmaAllocator allocator)
: buffer(buffer), allocation(allocation), allocator(allocator)
{

}

void Buffer::fill(const void *data, size_t bufferSize)
{
    void* mappedData;
    vmaMapMemory(allocator, allocation, &mappedData);
    memcpy(mappedData, &data, bufferSize);
    vmaUnmapMemory(allocator, allocation);
}

void Buffer::dispose()
{
    vmaDestroyBuffer(allocator, buffer, allocation);
}

void Buffer::bind(VkCommandBuffer commandBuffer)
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);
}
