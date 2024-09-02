#include <iostream>
#include "window.h"

Window::Window(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT) : SCR_WIDTH(SCR_WIDTH), SCR_HEIGHT(SCR_HEIGHT)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "qhenki renderer - Vulkan", nullptr, nullptr);
    // center the window
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(window, (mode->width - SCR_WIDTH) * 0.5, (mode->height - SCR_HEIGHT) * 0.5);
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::resize(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT)
{

}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(window);
}

vk::SurfaceKHR Window::createSurface(vk::Instance instance)
{
    VkSurfaceKHR s;
    if (glfwCreateWindowSurface(instance, window, nullptr, &s) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
    surface = vk::SurfaceKHR(s);
    return surface;
}

vk::SurfaceKHR Window::getSurface()
{
    return surface;
}

GLFWwindow *Window::getWindow()
{
    return window;
}
