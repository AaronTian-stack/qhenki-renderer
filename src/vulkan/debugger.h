#pragma once

#include "vulkan/vulkan.h"
#include "instance.h"

class Debugger
{
public:
    // returns creation info struct
    static vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo(bool verbose);

    static VkBool32
    debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData);
};
