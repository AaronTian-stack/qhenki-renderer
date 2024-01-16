#pragma once

#include <optional>
#include "glm/detail/type_mat4x4.hpp"

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

    struct CameraMatrices
    {
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 model;
    };
}

