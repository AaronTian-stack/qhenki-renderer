#include "bufferfactory.h"

void BufferFactory::create(VulkanContext &context)
{
    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorCreateInfo.physicalDevice = context.devicePicker.getPhysicalDevice();
    allocatorCreateInfo.device = context.devicePicker.getDevice();
    allocatorCreateInfo.instance = context.vulkanInstance.getInstance();
    allocatorCreateInfo.pVulkanFunctions = nullptr;

    vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

uPtr<Buffer> BufferFactory::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; // TODO: change this so you cam make other types of buffers

    VkBuffer buffer;
    VmaAllocation allocation;
    vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr);

    return mkU<Buffer>(buffer, allocation, allocator);
}

void BufferFactory::dispose()
{
    vmaDestroyAllocator(allocator);
}
