#include "vulkancontext.h"

void VulkanContext::create(Window &window)
{
    VulkanInstance::listExtensions();

    const bool verbose = false;
    vulkanInstance.create(verbose);
    debugger.create(vulkanInstance, verbose);
    window.createSurface(vulkanInstance);
    devicePicker.pickPhysicalDevice(vulkanInstance, window.getSurface());
    devicePicker.createLogicalDevice();
    // retrieve queues
    queueManager.initQueues(devicePicker.getDevice(), devicePicker.selectedDeviceFamily());

    // create swap chain (the render pass needs the image formats)
    // TODO: maybe refactor to extract properties before the swap chain creation (so that framebuffer creation is also part of createSwapChain)
    swapChain.createSwapChain(devicePicker, window);
}

void VulkanContext::dispose()
{
    // destroy swap chain before the device
    swapChain.dispose();
    // destroy logical device
    devicePicker.dispose();
    // destroy debugger
    debugger.dispose();
}
