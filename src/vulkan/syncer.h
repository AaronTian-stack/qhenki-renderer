#pragma once

#include <vulkan/vulkan.h>
#include <unordered_map>
#include "../disposable.h"

class Syncer : public Disposable
{
private:
    std::unordered_map<const char*, VkSemaphore> semaphores;
    std::unordered_map<const char*, VkFence> fences;

public:
    void create(VkDevice device);
    VkSemaphore createSemaphore();
    VkSemaphore createSemaphore(const char* name);

    VkFence createFence(bool startSignaled = false);
    VkFence createFence(const char* name, bool startSignaled = false);

    VkSemaphore getSemaphore(const char* name);
    VkFence getFence(const char* name);

    void resetFence(VkFence fence);
    void resetFence(const char* name);
    void waitForFence(const char* name);
    void waitForFence(VkFence fence);

    void dispose() override;
};
