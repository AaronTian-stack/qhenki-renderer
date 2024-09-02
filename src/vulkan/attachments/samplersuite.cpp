#include "samplersuite.h"

SamplerSuite::SamplerSuite(vk::Device device) : Destroyable(device)
{
    nearestSampler = createSampler(vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest);
    linearSampler = createSampler(vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);
}

void SamplerSuite::destroy()
{
    device.destroySampler(nearestSampler);
    device.destroySampler(linearSampler);
}

vk::Sampler SamplerSuite::createSampler(vk::Filter filter, vk::SamplerMipmapMode mipmapMode)
{
    vk::SamplerCreateInfo samplerInfo{
            vk::SamplerCreateFlags(),
            filter,
            filter,
            mipmapMode,
            vk::SamplerAddressMode::eClampToEdge,
            vk::SamplerAddressMode::eClampToEdge,
            vk::SamplerAddressMode::eClampToEdge,
            0.0f,
            VK_FALSE, // TODO: needs hardware support
            16,
            VK_FALSE,
            vk::CompareOp::eAlways,
            0.0f,
            0.0f,
            vk::BorderColor::eIntOpaqueBlack,
            VK_FALSE
    };
    return device.createSampler(samplerInfo);
}
