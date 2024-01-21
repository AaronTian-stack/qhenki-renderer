#pragma once

#include <vulkan/vulkan.hpp>
#include "../destroyable.h"

class DescriptorAllocator : public Destroyable
{
private:
    struct PoolSizes
    {
        std::vector<std::pair<vk::DescriptorType, float>> sizes =
                {
                        {vk::DescriptorType::eSampler,              0.5f},
                        {vk::DescriptorType::eCombinedImageSampler, 4.f},
                        {vk::DescriptorType::eSampledImage,         4.f},
                        {vk::DescriptorType::eStorageImage,         1.f},
                        {vk::DescriptorType::eUniformTexelBuffer,   1.f},
                        {vk::DescriptorType::eStorageTexelBuffer,   1.f},
                        {vk::DescriptorType::eUniformBuffer,        2.f},
                        {vk::DescriptorType::eStorageBuffer,        2.f},
                        {vk::DescriptorType::eUniformBufferDynamic, 1.f},
                        {vk::DescriptorType::eStorageBufferDynamic, 1.f},
                        {vk::DescriptorType::eInputAttachment,      0.5f}
                };
    } descriptorSizes;

    vk::DescriptorPool createPool(int count, vk::DescriptorPoolCreateFlags flags);
    vk::DescriptorPool grabPool();
    vk::DescriptorPool currentPool {VK_NULL_HANDLE};
    std::vector<vk::DescriptorPool> usedPools;
    std::vector<vk::DescriptorPool> freePools;

public:
    void resetPools();
    bool allocate(vk::DescriptorSet* set, vk::DescriptorSetLayout layout);

    void destroy() override;

    friend class DescriptorBuilder;
};

