#pragma once

#include "vulkan/vulkan.hpp"
#include "image.h"

class Texture : public Destroyable
{
    vk::Sampler sampler = nullptr;
    Image *image;

public:
    Texture(Image *image);
    void createSampler();
    void createSampler(vk::SamplerCreateInfo samplerInfo);
    vk::DescriptorImageInfo getDescriptorInfo();
    void destroy() override;
};
