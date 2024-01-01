#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <iostream>
#include "src/pathtracerapp.h"

Window window(800, 600);

int main()
{
    PathTracerApp app;
    app.create(window);

    while(!window.shouldClose())
    {
        glfwPollEvents();
        app.render();
    }

    app.dispose();
    window.destroySurface(app.getVulkanInstance());

    // vulkan instance is destroyed in destructor of app
    // glfw destroys window in destructor
    return 0;
}