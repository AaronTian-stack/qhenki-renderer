#pragma once

#include "vulkan/vulkan.h"
#include "vulkaninstance.h"

class VkDebugger : public Disposable
{
private:
    VkDebugUtilsMessengerEXT debugMessenger;
    VkInstance instance;

    /*
     * Creates a debug messenger manually since vkCreateDebugUtilsMessengerEXT is not an extension function.
     * Thus, it must be searched for manually.
     */
    VkResult createDebugUtilsMessengerEXT(
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger);

    void destroyDebugUtilsMessengerEXT(
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator);

public:
    VkDebugger();
    void create(VulkanInstance vkInstance, bool verbose);

    // returns creation info struct
    static VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo(bool verbose);

    static VkBool32
    debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData);

    void dispose() override;
};
