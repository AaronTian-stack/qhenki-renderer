#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

class Window
{
private:
    // window surface to interact with the window system
    vk::SurfaceKHR surface; // app must destroy this
    GLFWwindow* window;
    unsigned int SCR_WIDTH;
    unsigned int SCR_HEIGHT;

public:
    Window(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT);
    vk::SurfaceKHR createSurface(vk::Instance instance);
    // TODO: needs to be added as callback
    void resize(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT);
    bool shouldClose();
    ~Window();

    vk::SurfaceKHR getSurface();
    GLFWwindow* getWindow();
};
