#pragma once

#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "../disposable.h"

class Buffer : Disposable
{
private:
    VmaAllocator allocator;

public:
    Buffer(VkBuffer buffer, VmaAllocation allocation, VmaAllocator allocator);

    void fill(const void *data, size_t bufferSize);

    void bind(VkCommandBuffer commandBuffer);

    void dispose() override;

    VkBuffer buffer;
    VmaAllocation allocation;
};
