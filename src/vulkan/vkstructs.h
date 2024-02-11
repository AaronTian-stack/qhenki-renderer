#pragma once

#include <optional>

namespace VkStructs
{
    struct QueueFamilyIndices
    {
        // for drawing
        std::optional<uint32_t> graphicsFamily;
        // for presentation
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            // found a graphics queue family (with graphics bit) and a present queue family
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
}

