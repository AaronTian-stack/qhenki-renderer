#pragma once

#include <vulkan/vulkan.hpp>
#include "../destroyable.h"

class SamplerSuite : public Destroyable
{
private:
    vk::Sampler nearestSampler;
    vk::Sampler linearSampler;
    vk::Sampler createSampler(vk::Filter filter, vk::SamplerMipmapMode mipmapMode);

public:
    SamplerSuite(vk::Device device);
    inline vk::Sampler getNearestSampler() { return nearestSampler; }
    inline vk::Sampler getLinearSampler() { return linearSampler; }
    void destroy() override;
};
