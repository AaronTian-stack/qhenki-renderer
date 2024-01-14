#pragma once

#include "vulkan/vulkan.hpp"
#include "../destroyable.h"
#include <vector>

class Instance
{
private:
#ifdef NDEBUG
    static inline const bool enableValidationLayers = false;
#else
    static inline const bool enableValidationLayers = true;
#endif
    static bool checkValidationLayerSupport(std::vector<const char*> validationLayers);
    static std::vector<const char*> getRequiredExtensions();
public:

    static vk::Instance create(bool verbose);
    static void listExtensions();

};
