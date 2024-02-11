#pragma once

#include "../destroyable.h"

class DescriptorLayoutCache : public Destroyable
{
private:

    struct DescriptorLayoutInfo {
        //good idea to turn this into a inlined array
        std::vector<vk::DescriptorSetLayoutBinding> bindings;

        bool operator==(const DescriptorLayoutInfo& other) const;

        size_t hash() const;
    };

    struct DescriptorLayoutHash
    {

        std::size_t operator()(const DescriptorLayoutInfo& k) const
        {
            return k.hash();
        }
    };

    std::unordered_map<DescriptorLayoutInfo, vk::DescriptorSetLayout, DescriptorLayoutHash> layoutCache;

public:
    vk::DescriptorSetLayout createDescriptorLayout(vk::DescriptorSetLayoutCreateInfo* info);
    void destroy() override;

};
