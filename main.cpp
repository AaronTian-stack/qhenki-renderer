#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "src/vulkanapp.h"

Window window(1024, 768);

int main()
{
    // if on macOS you will need to set MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS=1 in environment variables
    VulkanApp app;
    app.create(window);

    UserInterface ui;
    auto param = app.getImGuiCreateParameters();
    param.window = &window;
    ui.create(param, app.getGraphicsCommandPool());

    app.ui = &ui;
    app.setUpCallbacks();

    while (!window.shouldClose()) {
        glfwPollEvents();
        if (glfwGetKey(window.getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            break;

        ui.begin();
        ui.render(app.getPartialMenuPayload());
        app.render(); // ui.end() called in here since command buffer is needed
    }

    ui.destroy();
    return 0;
}
