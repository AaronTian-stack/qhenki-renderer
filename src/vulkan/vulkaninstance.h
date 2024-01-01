#pragma once

#include "vulkan/vulkan.h"
#include "../disposable.h"
#include <vector>

class VulkanInstance : public Disposable
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
    void create(bool verbose);
    static void listExtensions();
    static bool checkValidationLayerSupport(std::vector<const char*> validationLayers);
    std::vector<const char*> getRequiredExtensions() const;
    void dispose() override;

    VkInstance getInstance() const;

};
