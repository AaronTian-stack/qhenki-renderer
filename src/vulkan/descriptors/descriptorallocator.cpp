#include "descriptorallocator.h"

vk::DescriptorPool DescriptorAllocator::createPool(int count, vk::DescriptorPoolCreateFlags flags)
{
    std::vector<vk::DescriptorPoolSize> sizes;
    sizes.reserve(descriptorSizes.sizes.size());
    for (auto sz : descriptorSizes.sizes) {
        sizes.emplace_back(sz.first, uint32_t(sz.second * count));
    }

    vk::DescriptorPoolCreateInfo poolInfo(
            flags,
            count,
            (uint32_t)sizes.size(),
            sizes.data()
            );

    return device.createDescriptorPool(poolInfo);
}

vk::DescriptorPool DescriptorAllocator::grabPool()
{
    if (!freePools.empty())
    {
        // grab pool from the back of the vector and remove it from there
        auto pool = freePools.back();
        freePools.pop_back();
        return pool;
    }
    else
    {
        // create new pool
        return createPool(1000, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    }
}

void DescriptorAllocator::destroy()
{
    for (auto& pool : usedPools)
    {
        device.destroyDescriptorPool(pool);
    }
    for (auto& pool : freePools)
    {
        device.destroyDescriptorPool(pool);
    }
}

void DescriptorAllocator::resetPools()
{
    for (auto p : usedPools)
    {
        device.resetDescriptorPool(p);
        freePools.push_back(p);
    }

    usedPools.clear();
    currentPool = VK_NULL_HANDLE;
}

bool DescriptorAllocator::allocate(vk::DescriptorSet *set, vk::DescriptorSetLayout layout)
{
    // initialize the currentPool handle if it's null
    if (currentPool == VK_NULL_HANDLE)
    {
        currentPool = grabPool();
        usedPools.push_back(currentPool);
    }

    vk::DescriptorSetAllocateInfo allocateInfo(
            currentPool,
            1,
            &layout
            );

    // try to allocate the descriptor set
    auto result = device.allocateDescriptorSets(&allocateInfo, set);

    bool needReallocate = false;

    switch (result)
    {
        case vk::Result::eSuccess:
            // good, return
            return true;
        case vk::Result::eErrorFragmentedPool:
        case vk::Result::eErrorOutOfPoolMemory:
            // reallocate pool
            needReallocate = true;
            break;
        default:
            // unrecoverable error
            return false;
    }

    if (needReallocate)
    {
        // allocate a new pool and retry
        currentPool = grabPool();
        usedPools.push_back(currentPool);

        result = device.allocateDescriptorSets(&allocateInfo, set);

        // if it still fails then we have big issues
        if (result == vk::Result::eSuccess)
        {
            return true;
        }
    }

    return false;
}

DescriptorAllocator::DescriptorAllocator(vk::Device device) : Destroyable(device)
{}
