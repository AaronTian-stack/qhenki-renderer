#include <stdexcept>
#include <iostream>
#include "GLFW/glfw3.h"
#include "debugger.h"
#include "instance.h"


vk::Instance Instance::create(bool verbose)
{
    const std::vector<const char*> validationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };

    if (enableValidationLayers && !checkValidationLayerSupport(validationLayers))
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    auto appInfo = vk::ApplicationInfo(
            "Vulkan Application",
            VK_MAKE_VERSION(1, 0, 0),
            "No Engine",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_2
            );

    vk::InstanceCreateInfo createInfo(
            {vk::InstanceCreateFlags(VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR)},
            &appInfo);

    auto extensions = getRequiredExtensions();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = Debugger::debugMessengerCreateInfo(verbose);

    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        // hook separate debug utils messenger into vkCreate and vkDestroy
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }

    return vk::createInstance(createInfo);
}

void Instance::listExtensions()
{
    auto extensions = vk::enumerateInstanceExtensionProperties();

    std::cout << "available instance extensions:\n";

    for (const auto &extension : extensions)
    {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

bool Instance::checkValidationLayerSupport(std::vector<const char *> validationLayers)
{
    auto availableLayers = vk::enumerateInstanceLayerProperties();

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char *> Instance::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // macOS requires the VK_KHR_portability_subset extension to be enabled.
    // needs to be done for every vk instance.
#ifdef __APPLE__
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
    extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}
