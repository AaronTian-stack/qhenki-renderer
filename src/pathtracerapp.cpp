#include "pathtracerapp.h"
#include "smartpointer.h"

PathTracerApp::PathTracerApp() : vulkanInstance() {}

PathTracerApp::~PathTracerApp() {}

void PathTracerApp::create()
{
    window = mkU<Window>(800, 600);
    vulkanInstance.listExtensions();
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
