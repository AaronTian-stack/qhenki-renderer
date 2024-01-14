#include <iostream>
#include "debugger.h"

VkBool32 Debugger::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
}

vk::DebugUtilsMessengerCreateInfoEXT Debugger::debugMessengerCreateInfo(bool verbose)
{
    auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT();

    // message severity filter
    createInfo.messageSeverity = (
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    if (verbose)
    {
        createInfo.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
    }

    createInfo.messageType = (
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);

    createInfo.pfnUserCallback = debugCallback;
    return createInfo;
}
