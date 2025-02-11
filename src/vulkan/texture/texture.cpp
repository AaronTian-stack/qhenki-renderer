#include "texture.h"

Texture::Texture(Image *image) : Destroyable(image->device), image(image)
{}

void Texture::createSampler()
{
    vk::SamplerCreateInfo samplerInfo{
            vk::SamplerCreateFlags(),
            vk::Filter::eLinear,
            vk::Filter::eLinear,
            vk::SamplerMipmapMode::eLinear,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            0.0f,
            VK_TRUE, // needs hardware support
            16,
            VK_FALSE,
            vk::CompareOp::eAlways,
            0.0f,
            1000.0f,
            vk::BorderColor::eIntOpaqueBlack,
            VK_FALSE
    };
    createSampler(samplerInfo);
}

void Texture::createSampler(vk::SamplerCreateInfo samplerInfo)
{
    if (sampler != nullptr)
        device.destroySampler(sampler);
    sampler = device.createSampler(samplerInfo);
}

vk::DescriptorImageInfo Texture::getDescriptorInfo()
{
    vk::DescriptorImageInfo info{};
    info.imageView = image->attachment->imageView;
    info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    info.sampler = sampler;
    return info;
}

void Texture::destroy()
{
    // multiple textures can share the same image
    if (!image->destroyed)
        image->destroy();
    image->destroyed = true;
    if (sampler != nullptr)
        device.destroySampler(sampler);
}
