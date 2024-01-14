#include "vulkancontext.h"
#include "debugger.h"

void VulkanContext::create(Window &window)
{
    const bool verbose = false;

    Instance::listExtensions();

    instance = Instance::create(verbose);

    auto dy = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
    debugger = instance.createDebugUtilsMessengerEXT(
            Debugger::debugMessengerCreateInfo(verbose),
            nullptr, dy);

    auto surface = window.createSurface(instance);
    this->window = &window;

    device = DevicePicker::createDevice(instance, surface);

    queueManager.initQueues(device.logicalDevice, device.queueFamilyIndices);

    swapChain.createSwapChain(device, window); // does not create framebuffers. that comes after the renderpass

    // TODO: maybe refactor to extract properties before the swap chain creation (so that framebuffer creation is also part of createSwapChain)
}

VulkanContext::~VulkanContext()
{
    swapChain.destroy();
    device.destroy();

    //destroy debugger
    auto dy = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
    instance.destroyDebugUtilsMessengerEXT(debugger, nullptr, dy);
    instance.destroySurfaceKHR(window->getSurface());

    // destroy instance last
    instance.destroy();
}
