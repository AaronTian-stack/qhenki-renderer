#include "pathtracerapp.h"

PathTracerApp::PathTracerApp()
{

}

PathTracerApp::~PathTracerApp()
{
    // destroy instance last
    vulkanInstance.destroy();
}

void PathTracerApp::destroy()
{
    // destroy swap chain before the device
    swapChainManager.destroy(vulkanDevicePicker);
    // destroy logical device
    vulkanDevicePicker.destroy();
    // destroy debugger
    vulkanDebugger.destroy(vulkanInstance);
}

void PathTracerApp::create(Window &window)
{
    VulkanInstance::listExtensions();

    // create instance, load extensions
    vulkanInstance.create(false);
    // create debugger, validation layers
    vulkanDebugger.create(vulkanInstance, false);
    window.createSurface(vulkanInstance);
    // pick physical device (with feature checking)
    vulkanDevicePicker.pickPhysicalDevice(vulkanInstance, window.getSurface());
    // create queues, logical device
    vulkanDevicePicker.createLogicalDevice();
    // retrieve queues
    vulkanQueueManager.initQueues(vulkanDevicePicker.getDevice(), vulkanDevicePicker.selectedDeviceFamily());

    // create swap chain
    swapChainManager.createSwapChain(vulkanDevicePicker, window);
}

void PathTracerApp::render()
{

}

void PathTracerApp::resize()
{

}

VulkanInstance PathTracerApp::getVulkanInstance()
{
    return vulkanInstance;
}
