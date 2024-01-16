#include "syncer.h"

vk::Semaphore Syncer::createSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreInfo;
    return device.createSemaphore(semaphoreInfo);
}

vk::Semaphore Syncer::createSemaphore(const char* name)
{
    if (semaphores.find(name) != semaphores.end())
    {
        throw std::runtime_error("already existing semaphore name!");
    }
    auto sem = createSemaphore();
    semaphores[name] = sem;
    return sem;
}

vk::Fence Syncer::createFence(bool startSignaled)
{
    vk::FenceCreateInfo fenceInfo;
    if (startSignaled) fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    return device.createFence(fenceInfo);
}

vk::Fence Syncer::createFence(const char* name, bool startSignaled)
{
    if (fences.find(name) != fences.end())
    {
        throw std::runtime_error("already existing fence name!");
    }
    auto fence = createFence(startSignaled);
    fences[name] = fence;
    return fence;
}

vk::Semaphore Syncer::getSemaphore(const char *name)
{
    return semaphores[name];
}

vk::Fence Syncer::getFence(const char* name)
{
    return fences[name];
}

void Syncer::resetFence(vk::Fence fence)
{
    device.resetFences(fence);
}

void Syncer::resetFence(const char* name)
{
    resetFence(fences[name]);
}

vk::Result Syncer::waitForFence(vk::Fence fence)
{
    return device.waitForFences(fence, VK_TRUE, UINT64_MAX);
}

void Syncer::waitForFence(const char *name)
{
    auto fence = fences[name];
    waitForFence(fence);
}

void Syncer::destroy()
{
    for (auto semaphore : semaphores)
    {
        device.destroySemaphore(semaphore.second); // see vkDestroySemaphore
    }
    for (auto fence : fences)
    {
        device.destroyFence(fence.second); // see vkDestroyFence
    }
}
