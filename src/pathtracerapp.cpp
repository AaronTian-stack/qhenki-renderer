#include "pathtracerapp.h"
#include "smartpointer.h"

PathTracerApp::PathTracerApp()
{
    vulkanInstance.create();
    vulkanDebugger.create(vulkanInstance);
    vulkanDevicePicker.pickPhysicalDevice(vulkanInstance);
}

PathTracerApp::~PathTracerApp()
{
    vulkanDebugger.destroy(vulkanInstance);
    vulkanInstance.destroy();
    // device picker is implicitly destroyed with vulkan instance
}

void PathTracerApp::create()
{
    VulkanInstance::listExtensions();
}

void PathTracerApp::render()
{

}

void PathTracerApp::resize()
{

}
