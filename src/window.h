#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"
#include "vulkan/instance.h"

class Window {
private:
    // window surface to interact with the window system
    VkSurfaceKHR surface;
    GLFWwindow* window;
    unsigned int SCR_WIDTH;
    unsigned int SCR_HEIGHT;

public:
    Window(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT);
    void createSurface(VulkanInstance vkInstance);
    // TODO: needs to be added as callback
    void resize(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT);
    bool shouldClose();
    void destroySurface(VulkanInstance vkInstance);
    ~Window();

    VkSurfaceKHR getSurface();
    GLFWwindow* getWindow();
};
