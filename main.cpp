#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <iostream>
#include "src/pathtracerapp.h"

Window window(1024, 768);

int main()
{
    PathTracerApp app;
    app.create(window);

    UserInterface ui;
    auto param = app.getImGuiCreateParameters();
    param.window = &window;
    ui.create(param, app.getCommandPool());

    app.ui = &ui;

    while (!window.shouldClose())
    {
        glfwPollEvents();

        ui.begin();
        ui.render();
        app.render(); // ui.end() called in here since command buffer is needed
    }

    //ui.destroy(); // destructor does this
    return 0;
}