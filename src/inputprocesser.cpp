#include "inputprocesser.h"
#include "camera.h"
#include <iostream>

void InputProcesser::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (!mouseButtons[GLFW_MOUSE_BUTTON_1] || !mouseButtons[GLFW_MOUSE_BUTTON_2])
    {
        // set lastMousePos to current mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        lastMousePos = {static_cast<float>(xpos), static_cast<float>(ypos)};
    }
    mouseButtons[button] = action;
}

void InputProcesser::mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    auto xPos = static_cast<float>(xposIn);
    auto yPos = static_cast<float>(yposIn);

    float xOffset = xPos - lastMousePos.x;
    float yOffset = lastMousePos.y - yPos; // reversed since y-coordinates go from bottom to top

    lastMousePos = {xPos, yPos};

    deltaMouse = {xOffset, yOffset};

    // update camera based of glfw user pointer
    auto* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (cam)
        cam->update();
}

void InputProcesser::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto* obj = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (obj)
        obj->zoom(yoffset);
}

void InputProcesser::setCallbacks(Window &window)
{
    InputProcesser::window = window.getWindow();
    glfwSetCursorPosCallback(InputProcesser::window, mouse_callback);
    glfwSetMouseButtonCallback(InputProcesser::window, mouse_button_callback);
    glfwSetScrollCallback(InputProcesser::window, scroll_callback);
}

void InputProcesser::disableCursor()
{
    glfwSetInputMode(InputProcesser::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputProcesser::enableCursor()
{
    glfwSetInputMode(InputProcesser::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    deltaMouse = {0, 0};
}

int InputProcesser::getCursorState()
{
    return glfwGetInputMode(InputProcesser::window, GLFW_CURSOR);
}

void InputProcesser::setUserPointer(void *ptr)
{
    glfwSetWindowUserPointer(InputProcesser::window, ptr);
}
