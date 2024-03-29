#include "descriptorlayoutcache.h"

bool
DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutCache::DescriptorLayoutInfo &other) const
{
    if (other.bindings.size() != bindings.size())
    {
        return false;
    }
    else
    {
        // compare each of the bindings is the same. Bindings are sorted so they will match
        for (int i = 0; i < bindings.size(); i++)
        {
            if (other.bindings[i].binding != bindings[i].binding)
            {
                return false;
            }
            if (other.bindings[i].descriptorType != bindings[i].descriptorType)
            {
                return false;
            }
            if (other.bindings[i].descriptorCount != bindings[i].descriptorCount)
            {
                return false;
            }
            if (other.bindings[i].stageFlags != bindings[i].stageFlags)
            {
                return false;
            }
        }
        return true;
    }
}

size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
{
    size_t result = std::hash<size_t>()(bindings.size());

    for (const vk::DescriptorSetLayoutBinding &b : bindings)
    {
        size_t binding_hash = std::hash<size_t>()(b.binding);

        // shuffle the packed binding data and xor it with the main hash
        result ^= std::hash<size_t>()(binding_hash);
    }

    return result;
}

vk::DescriptorSetLayout DescriptorLayoutCache::createDescriptorLayout(vk::DescriptorSetLayoutCreateInfo *info)
{
    DescriptorLayoutInfo layoutInfo;
    layoutInfo.bindings.reserve(info->bindingCount);
    bool isSorted = true;
    int lastBinding = -1;

    // copy from the direct info struct into our own one
    for (int i = 0; i < info->bindingCount; i++)
    {
        layoutInfo.bindings.push_back(info->pBindings[i]);

        //check that the bindings are in strict increasing order
        if (info->pBindings[i].binding > lastBinding)
        {
            lastBinding = info->pBindings[i].binding;
        }
        else
        {
            isSorted = false;
        }
    }
    // sort the bindings if they aren't in order
    if (!isSorted)
    {
        std::sort(layoutInfo.bindings.begin(), layoutInfo.bindings.end(), [](vk::DescriptorSetLayoutBinding &a, vk::DescriptorSetLayoutBinding &b) {
            return a.binding < b.binding;
        });
    }

    // try to grab from cache
    auto it = layoutCache.find(layoutInfo);
    if (it != layoutCache.end())
    {
        return (*it).second;
    }
    else
    {
        vk::DescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
        extendedInfo.bindingCount = layoutInfo.bindings.size();
        // update after bind is not very feasible due to limitations on hardware ie NV pascal
        // but enable partial binding to silence errors
        vk::DescriptorBindingFlags test = vk::DescriptorBindingFlagBits::ePartiallyBound;
        extendedInfo.pBindingFlags = &test;
        info->pNext = &extendedInfo;

        // create a new one (not found)
        vk::DescriptorSetLayout layout = device.createDescriptorSetLayout(*info);
        // add to cache
        layoutCache[layoutInfo] = layout;
        return layout;
    }
}

void DescriptorLayoutCache::destroy()
{
    for (auto &pair : layoutCache)
    {
        device.destroyDescriptorSetLayout(pair.second);
    }
    layoutCache.clear();
}
