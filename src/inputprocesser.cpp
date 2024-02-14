#include "inputprocesser.h"
#include "camera.h"
#include <iostream>

void InputProcesser::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
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
    auto* obj = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (obj)
        obj->update(0.0f);
}

void InputProcesser::setCallbacks(Window &window)
{
    InputProcesser::window = window.getWindow();
    glfwSetCursorPosCallback(InputProcesser::window, mouse_callback);
    glfwSetMouseButtonCallback(InputProcesser::window, mouse_button_callback);
}

void InputProcesser::disableCursor()
{
    glfwSetInputMode(InputProcesser::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputProcesser::enableCursor()
{
    glfwSetInputMode(InputProcesser::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

int InputProcesser::getCursorState()
{
    return glfwGetInputMode(InputProcesser::window, GLFW_CURSOR);
}
