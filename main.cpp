#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <iostream>
#include "src/vulkanapp.h"

Window window(1366, 768);

int main()
{
    VulkanApp app;
    app.create(window);

    UserInterface ui;
    auto param = app.getImGuiCreateParameters();
    param.window = &window;
    ui.create(param, app.getCommandPool());

    app.ui = &ui;

    while (!window.shouldClose()) {
        glfwPollEvents();
        if (glfwGetKey(window.getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            break;

        ui.begin();
        ui.render();
        app.render(); // ui.end() called in here since command buffer is needed
    }

    //ui.destroy(); // destructor does this
    return 0;
}
