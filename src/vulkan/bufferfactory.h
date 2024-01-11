#pragma once

#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "vulkancontext.h"
#include "buffer.h"
#include "../smartpointer.h"

class BufferFactory
{
private:
    VmaAllocator allocator;

public:
    void create(VulkanContext &context);

    uPtr<Buffer> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

    void dispose();
};
