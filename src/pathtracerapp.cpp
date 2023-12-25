#include "pathtracerapp.h"
#include "smartpointer.h"

PathTracerApp::PathTracerApp()
{
    vulkanInstance.create();
    vulkanDebugger.create(vulkanInstance);
}

PathTracerApp::~PathTracerApp()
{
    vulkanDebugger.destroy(vulkanInstance);
    vulkanInstance.destroy();
}

void PathTracerApp::create()
{
    window = mkU<Window>(800, 600);
    VulkanInstance::listExtensions();
}

void PathTracerApp::render()
{

}

void PathTracerApp::resize()
{

}

bool PathTracerApp::isAlive()
{
    return !window->shouldClose();
}
