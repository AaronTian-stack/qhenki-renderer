#include "inputprocesser.h"
#include "camera.h"

void InputProcesser::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (!buttons[GLFW_MOUSE_BUTTON_1] || !buttons[GLFW_MOUSE_BUTTON_2])
    {
        // set lastMousePos to current mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        lastMousePos = {static_cast<float>(xpos), static_cast<float>(ypos)};
    }
    if (button == GLFW_MOUSE_BUTTON_4 || button == GLFW_MOUSE_BUTTON_5)
    {
        float adjust = button == GLFW_MOUSE_BUTTON_4 ? SENSITIVITY_FOV : -SENSITIVITY_FOV;
        auto* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (cam)
        {
            if (action == GLFW_PRESS)
                cam->adjustFOV(adjust);
        }
    }
    buttons[button] = action;
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

    // lock mouse position
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glfwSetCursorPos(window, width / 2, height / 2);
        lastMousePos = {width / 2, height / 2};
    }
}

void InputProcesser::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (cam)
        cam->zoom(yoffset);
}

void InputProcesser::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    buttons[key] = action;
}

void InputProcesser::setCallbacks(Window &window)
{
    InputProcesser::window = window.getWindow();
    glfwSetCursorPosCallback(InputProcesser::window, mouse_callback);
    glfwSetMouseButtonCallback(InputProcesser::window, mouse_button_callback);
    glfwSetScrollCallback(InputProcesser::window, scroll_callback);
    glfwSetKeyCallback(InputProcesser::window, key_callback);

    glfwSetWindowUserPointer(InputProcesser::window, nullptr);
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

void *InputProcesser::getUserPointer()
{
    return glfwGetWindowUserPointer(InputProcesser::window);
}

int InputProcesser::getButton(int key)
{
    return buttons[key];
}
