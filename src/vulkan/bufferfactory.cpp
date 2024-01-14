#include "bufferfactory.h"

void BufferFactory::create(VulkanContext &context)
{
    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorCreateInfo.physicalDevice = context.device.physicalDevice;
    allocatorCreateInfo.device = context.device.logicalDevice;
    allocatorCreateInfo.instance = context.instance;
    allocatorCreateInfo.pVulkanFunctions = nullptr;

    vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

uPtr<Buffer> BufferFactory::createBuffer(vk::DeviceSize size, VkBufferUsageFlags usage)
{
    // must use C api with VMA
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VkBuffer buffer;
    VmaAllocation allocation;
    if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer");

    return mkU<Buffer>(vk::Buffer(buffer), allocation, allocator);
}

void BufferFactory::destroy()
{
    vmaDestroyAllocator(allocator);
}
