#pragma once

#include "descriptorlayoutcache.h"
#include "descriptorallocator.h"

class DescriptorBuilder
{
private:
    std::vector<vk::WriteDescriptorSet> writes;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    
    DescriptorLayoutCache *cache;
    DescriptorAllocator *alloc;
    
public:
    static DescriptorBuilder beginSet(DescriptorLayoutCache *layoutCache, DescriptorAllocator *allocator);

    DescriptorBuilder& bindBuffer(uint32_t binding, vk::DescriptorBufferInfo *bufferInfo, vk::DescriptorType type, vk::ShaderStageFlags stageFlags);

    DescriptorBuilder& bindImage(uint32_t binding, vk::DescriptorImageInfo *imageInfo, vk::DescriptorType type, vk::ShaderStageFlags stageFlags);

    bool build(vk::DescriptorSet &set, vk::DescriptorSetLayout &layout);
    bool build(vk::DescriptorSet &set);
};
