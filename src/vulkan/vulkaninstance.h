#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class VulkanInstance
{
private:
    VkInstance instance;
public:

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    VulkanInstance();
    void create();
    static void listExtensions();
    static bool checkValidationLayerSupport(std::vector<const char*> validationLayers);
    std::vector<const char*> getRequiredExtensions() const;
    void destroy();

    VkInstance getInstance() const;

};
