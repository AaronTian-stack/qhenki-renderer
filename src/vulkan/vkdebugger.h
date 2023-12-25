#include "vulkan/vulkan.h"

class VKDebugger
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

public:
    VKDebugger();
    ~VKDebugger();

    static VkBool32
    debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);
};
