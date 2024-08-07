#include "descriptorbuilder.h"

DescriptorBuilder DescriptorBuilder::beginSet(DescriptorLayoutCache *layoutCache, DescriptorAllocator *allocator)
{
    DescriptorBuilder builder;
    builder.cache = layoutCache;
    builder.alloc = allocator;
    return builder;
}

DescriptorBuilder&
DescriptorBuilder::bindBuffer(uint32_t binding, vk::DescriptorBufferInfo *bufferInfo, vk::DescriptorType type,
                              vk::ShaderStageFlags stageFlags)
{
    vk::DescriptorSetLayoutBinding newBinding(
            binding,
            type,
            1,
            stageFlags,
            nullptr
            );
    bindings.push_back(newBinding);

    vk::WriteDescriptorSet newWrite(
            nullptr, // needs to be set when building
            binding,
            0, // index to start at
            1, // array elements to update
            type,
            nullptr,
            bufferInfo,
            nullptr
            );

    writes.push_back(newWrite);
    return *this;
}

DescriptorBuilder &
DescriptorBuilder::bindImage(uint32_t binding, const std::vector<vk::DescriptorImageInfo> &imageInfos, uint32_t arraySize,
                             vk::DescriptorType type, vk::ShaderStageFlags stageFlags)
{
    vk::DescriptorSetLayoutBinding newBinding(
            binding,
            type,
            arraySize, // size of array in shader. needed for correct map lookup
            stageFlags,
            nullptr
    );
    bindings.push_back(newBinding);

    vk::WriteDescriptorSet newWrite(
            nullptr, // needs to be set when building
            binding,
            0, // index to start at
            imageInfos.size(), // array elements to update
            type,
            imageInfos.data(),
            nullptr,
            nullptr
            );

    writes.push_back(newWrite);
    return *this;
}

bool DescriptorBuilder::build(vk::DescriptorSet &set, vk::DescriptorSetLayout &layout)
{
    // build layout
    vk::DescriptorSetLayoutCreateInfo layoutInfo(
            vk::DescriptorSetLayoutCreateFlags(),
            static_cast<uint32_t>(bindings.size()),
            bindings.data()
            );

    layout = cache->createDescriptorLayout(&layoutInfo); // find or create which set layout this is based off the bindings

    // allocate descriptor set into set from layout
    bool success = alloc->allocate(&set, layout);
    if (!success) { return false; }

    // set destination set for all writes
    for (auto &w : writes)
    {
        w.dstSet = set;
    }

    alloc->device.updateDescriptorSets(writes, nullptr);

    bindings.clear();
    writes.clear();

    return true;
}

bool DescriptorBuilder::build(vk::DescriptorSet &set)
{
    vk::DescriptorSetLayout layout;
    return build(set, layout);
}
