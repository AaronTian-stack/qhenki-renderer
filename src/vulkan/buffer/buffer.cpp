#include "buffer.h"
#include "bufferfactory.h"
#include <iostream>

Buffer::Buffer(vk::Buffer buffer, vk::BufferCreateInfo info, VmaAllocation allocation, VmaAllocator allocator, bool persistent)
: buffer(buffer), info(info), allocation(allocation), allocator(allocator), persistent(persistent), pointerMapped(false)
{
    if (persistent)
    {
        vmaMapMemory(allocator, allocation, &mappedData);
    }
    if (info.usage & vk::BufferUsageFlagBits::eIndexBuffer)
    {
        // default value
        indexType = vk::IndexType::eUint16;
    }
}

void Buffer::fill(const void *data, unsigned int offset, vk::DeviceSize size)
{
    if (persistent)
    {
        char *p = static_cast<char *>(mappedData) + offset;
        memcpy(p + offset, data, size);
    }
    else
    {
        vmaMapMemory(allocator, allocation, &mappedData);
        char *p = static_cast<char *>(mappedData);
        memcpy(p + offset, data, size);
        vmaUnmapMemory(allocator, allocation);
    }
}

void Buffer::fill(const void *data)
{
    fill(data, 0, info.size);
}

void Buffer::destroy()
{
    if (persistent)
    {
        vmaUnmapMemory(allocator, allocation);
    }
    vmaDestroyBuffer(allocator, buffer, allocation);
}

void Buffer::bind(vk::CommandBuffer commandBuffer, int binding)
{
    VkDeviceSize offset = 0;
    if (info.usage & vk::BufferUsageFlagBits::eIndexBuffer)
    {
        if (indexType == std::nullopt)
            throw std::runtime_error("Index type not set");
        commandBuffer.bindIndexBuffer(buffer, offset, indexType.value());
    }
    else if (info.usage & vk::BufferUsageFlagBits::eVertexBuffer)
        commandBuffer.bindVertexBuffers(binding, 1, &buffer, &offset);
}

void Buffer::copyTo(Buffer &destination, QueueManager &queueManager, CommandPool &commandPool)
{
    auto commandBuffer = commandPool.beginSingleCommand();

    vk::BufferCopy copyRegion(0, 0, this->info.size);
    commandBuffer.copyBuffer(this->buffer, destination.buffer, 1, &copyRegion);

    commandBuffer.end();
    commandPool.submitSingleTimeCommands(queueManager, {commandBuffer},true);
}

void Buffer::setIndexType(vk::IndexType type)
{
    indexType = type;
}

std::optional<vk::IndexType> Buffer::getIndexType()
{
    return indexType;
}

vk::DescriptorBufferInfo Buffer::getDescriptorInfo()
{
    return {buffer, 0, info.size};
}

void *Buffer::getPointer()
{
    if (pointerMapped)
        throw std::runtime_error("Buffer already mapped");
    if (!persistent)
    {
        vmaMapMemory(allocator, allocation, &mappedData);
        pointerMapped = true;
    }
    return mappedData;
}

void Buffer::unmap()
{
    if (!persistent)
    {
        if (!pointerMapped)
            throw std::runtime_error("Buffer not mapped");
        vmaUnmapMemory(allocator, allocation);
        pointerMapped = false;
    }
    else
        std::cerr << "unmapping persistent buffer" << std::endl;
}

void bind(vk::CommandBuffer commandBuffer, const std::vector<Buffer*> &buffers)
{
    for (auto &buffer : buffers)
    {
        if (!(buffer->info.usage & vk::BufferUsageFlagBits::eVertexBuffer))
            throw std::runtime_error("Buffer is not a vertex buffer");
    }
    // collect vector of raw buffer objects
    std::vector<vk::Buffer> rawBuffers;
    rawBuffers.reserve(buffers.size());
    for (auto &buffer : buffers)
    {
        rawBuffers.push_back(buffer->buffer);
    }
    std::vector<vk::DeviceSize> offsets(rawBuffers.size());
    commandBuffer.bindVertexBuffers(0, rawBuffers.size(), rawBuffers.data(), offsets.data());
}
