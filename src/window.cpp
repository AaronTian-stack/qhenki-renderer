#include <iostream>
#include "window.h"

Window::Window(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT) : SCR_WIDTH(SCR_WIDTH), SCR_HEIGHT(SCR_HEIGHT)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Vulkan", nullptr, nullptr);
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

void Window::destroySurface(VulkanInstance vkInstance)
{
    vkDestroySurfaceKHR(vkInstance.getInstance(), surface, nullptr);
}

void Window::createSurface(VulkanInstance vkInstance)
{
    if (glfwCreateWindowSurface(vkInstance.getInstance(), window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

VkSurfaceKHR Window::getSurface()
{
    return surface;
}
