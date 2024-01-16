#pragma once

#include <vulkan/vulkan.h>
#include <unordered_map>
#include "../destroyable.h"

class Syncer : public Destroyable
{
private:
    std::unordered_map<const char*, vk::Semaphore> semaphores;
    std::unordered_map<const char*, vk::Fence> fences;

public:
    vk::Semaphore createSemaphore();
    vk::Semaphore createSemaphore(const char* name);

    vk::Fence createFence(bool startSignaled = false);
    vk::Fence createFence(const char* name, bool startSignaled = false);

    vk::Semaphore getSemaphore(const char* name);
    vk::Fence getFence(const char* name);

    void resetFence(vk::Fence fence);
    void resetFence(const char* name);
    void waitForFence(const char* name);
    vk::Result waitForFence(vk::Fence fence);



    void destroy() override;
};
