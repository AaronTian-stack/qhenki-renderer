#pragma once

#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "../destroyable.h"
#include "../commandpool.h"

class Buffer
{
private:
    const VmaAllocator allocator;
    const VmaAllocation allocation;
  
    void* mappedData;
    const bool persistent;

    std::optional<vk::IndexType> indexType;

public:
    Buffer(vk::Buffer buffer, vk::BufferCreateInfo info, VmaAllocation allocation, VmaAllocator allocator, bool persistent);

    void fill(const void *data);
    void copyTo(Buffer &destination, QueueManager &queueManager, CommandPool &commandPool);

    void bind(vk::CommandBuffer commandBuffer, int binding = 0);

    void destroy();

    void setIndexType(vk::IndexType type);
    std::optional<vk::IndexType> getIndexType();

    vk::Buffer buffer;
    vk::BufferCreateInfo info;
};

// binds sequentially in order
void bind(vk::CommandBuffer commandBuffer, const std::vector<Buffer*> &buffers);
