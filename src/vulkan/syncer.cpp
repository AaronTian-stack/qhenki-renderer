#include "syncer.h"

void Syncer::create(VkDevice device)
{
    deviceForDispose = device;
}

VkSemaphore Syncer::createSemaphore()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    auto sem = VkSemaphore();
    if (vkCreateSemaphore(deviceForDispose, &semaphoreInfo, nullptr, &sem) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create semaphores!");
    }
    return sem;
}

VkSemaphore Syncer::createSemaphore(const char* name)
{
    if (semaphores.find(name) != semaphores.end())
    {
        throw std::runtime_error("already existing semaphore name!");
    }
    auto sem = createSemaphore();
    semaphores[name] = sem;
    return sem;
}

VkFence Syncer::createFence(bool startSignaled)
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (startSignaled) fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    auto fence = VkFence();
    if (vkCreateFence(deviceForDispose, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create fences!");
    }
    return fence;
}

VkFence Syncer::createFence(const char* name, bool startSignaled)
{
    if (fences.find(name) != fences.end())
    {
        throw std::runtime_error("already existing fence name!");
    }
    auto fence = createFence(startSignaled);
    fences[name] = fence;
    return fence;
}

VkSemaphore Syncer::getSemaphore(const char *name)
{
    return semaphores[name];
}

VkFence Syncer::getFence(const char* name)
{
    return fences[name];
}

void Syncer::resetFence(VkFence fence)
{
    vkResetFences(deviceForDispose, 1, &fence);
}

void Syncer::resetFence(const char* name)
{
    resetFence(fences[name]);
}

void Syncer::waitForFence(VkFence fence)
{
    vkWaitForFences(deviceForDispose, 1, &fence, VK_TRUE, UINT64_MAX);
}

void Syncer::waitForFence(const char *name)
{
    auto fence = fences[name];
    waitForFence(fence);
}

void Syncer::dispose()
{
    for (auto semaphore : semaphores)
    {
        vkDestroySemaphore(deviceForDispose, semaphore.second, nullptr);
    }
    for (auto fence : fences)
    {
        vkDestroyFence(deviceForDispose, fence.second, nullptr);
    }
}
