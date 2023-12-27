#pragma once

#include "vulkan/vulkan.h"
#include "vulkaninstance.h"

class VkDebugger
{
private:
    VkDebugUtilsMessengerEXT debugMessenger;

    /*
     * Creates a debug messenger manually since vkCreateDebugUtilsMessengerEXT is not an extension function.
     * Thus, it must be searched for manually.
     */
    static VkResult createDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger);

    static void destroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator);

public:
    VkDebugger();
    void create(VulkanInstance vkInstance);

    static VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo();

    static VkBool32
    debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData);

    void destroy(const VulkanInstance &vkInstance);
};
