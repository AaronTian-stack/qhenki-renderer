#pragma once

#include <vulkan/vulkan.hpp>

class Destroyable
{
protected:
    vk::Device device;

public:
    Destroyable() {}
    Destroyable(vk::Device device) : device(device) {}

    virtual void create(vk::Device device) { this->device = device; }
    virtual void destroy() = 0;
};
