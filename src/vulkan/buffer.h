#pragma once

#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "../destroyable.h"

class Buffer
{
private:
    VmaAllocator allocator;

public:
    Buffer(vk::Buffer buffer, VmaAllocation allocation, VmaAllocator allocator);

    void fill(const void *data, size_t bufferSize);

    void bind(vk::CommandBuffer commandBuffer);

    void destroy();

    vk::Buffer buffer;
    VmaAllocation allocation;
};
