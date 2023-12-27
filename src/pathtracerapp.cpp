#include "pathtracerapp.h"

PathTracerApp::PathTracerApp()
{

}

PathTracerApp::~PathTracerApp()
{
    vulkanDevicePicker.destroy();
    vulkanDebugger.destroy(vulkanInstance);
    vulkanInstance.destroy();
}

void PathTracerApp::create(Window &window)
{
    VulkanInstance::listExtensions();

    // create instance, load extensions
    vulkanInstance.create();
    // create debugger, validation layers
    vulkanDebugger.create(vulkanInstance);
    window.createSurface(vulkanInstance);
    // pick physical device (with feature checking)
    vulkanDevicePicker.pickPhysicalDevice(vulkanInstance, window.getSurface());
    vulkanDevicePicker.createLogicalDevice();
    vulkanQueueManager.initQueues(vulkanDevicePicker.getDevice(), vulkanDevicePicker.selectedDeviceFamily());
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
