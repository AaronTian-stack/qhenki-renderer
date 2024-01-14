#pragma once
#include <vulkan/vulkan.h>
#include "vulkan/device.h"

class Destroyable
{
protected:
    vk::Device device;

public:
    Destroyable() {}
    Destroyable(vk::Device device) : device(device) {}
    virtual void destroy() = 0;
};
