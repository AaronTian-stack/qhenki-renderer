#pragma once

#include <vulkan/vulkan.h>
#include <unordered_map>
#include "destroyable.h"
#include <tsl/robin_map.h>

class Syncer : public Destroyable
{
private:
    tsl::robin_map<const char*, vk::Semaphore> semaphores;
    tsl::robin_map<const char*, vk::Fence> fences;

public:
    vk::Semaphore createSemaphore();
    vk::Semaphore createSemaphore(const char *name);

    vk::Fence createFence(bool startSignaled = false);
    vk::Fence createFence(const char *name, bool startSignaled = false);

    vk::Semaphore getSemaphore(const char *name);
    vk::Fence getFence(const char *name);

    void resetFence(vk::Fence fence);
    void resetFence(const char *name);
    void waitForFence(const char *name);
    vk::Result waitForFences(std::vector<vk::Fence> fence);



    void destroy() override;
};
