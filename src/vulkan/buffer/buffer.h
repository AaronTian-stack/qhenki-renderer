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

    bool pointerMapped; // for raw pointer modification tracking

    std::optional<vk::IndexType> indexType;

public:
    const vk::Buffer buffer;
    const vk::BufferCreateInfo info;

    Buffer(vk::Buffer buffer, vk::BufferCreateInfo info, VmaAllocation allocation, VmaAllocator allocator, bool persistent);

    void fill(const void *data);
    void fill(const void *data, unsigned int offset, vk::DeviceSize size);

    void* getPointer();
    void unmap();

    void copyTo(Buffer &destination, QueueManager &queueManager, CommandPool &commandPool);

    void bind(vk::CommandBuffer commandBuffer, int binding = 0);

    void destroy();

    void setIndexType(vk::IndexType type);
    std::optional<vk::IndexType> getIndexType();

    vk::DescriptorBufferInfo getDescriptorInfo();
};

// binds sequentially in order
void bind(vk::CommandBuffer commandBuffer, const std::vector<Buffer*> &buffers);
