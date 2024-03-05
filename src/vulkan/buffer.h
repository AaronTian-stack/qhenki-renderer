#pragma once

#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "destroyable.h"
#include "commandpool.h"

class Buffer
{
private:
    const VmaAllocator allocator;
    const VmaAllocation allocation;
    void* mappedData;
    const bool persistent;

public:
    Buffer(vk::Buffer buffer, vk::BufferCreateInfo info, VmaAllocation allocation, VmaAllocator allocator, bool persistent);

    void fill(const void *data);
    void copyTo(Buffer &destination, QueueManager &queueManager, CommandPool &commandPool);

    void bind(vk::CommandBuffer commandBuffer);

    void destroy();

    vk::Buffer buffer;
    vk::BufferCreateInfo info;
};

void bind(vk::CommandBuffer commandBuffer, const std::vector<Buffer>& buffers);
