#include "inputprocesser.h"
#include <iostream>

void InputProcesser::mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    auto xPos = static_cast<float>(xposIn);
    auto yPos = static_cast<float>(yposIn);

    float xOffset = xPos - lastMousePos.x;
    float yOffset = lastMousePos.y - yPos; // reversed since y-coordinates go from bottom to top
    deltaMouse = {xOffset, yOffset};

    for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
        mouseButtons[i] = glfwGetMouseButton(window, i);

    std::cout << "Mouse moved to: " << xPos << ", " << yPos << std::endl;
}

void InputProcesser::create(Window &window)
{
    glfwSetCursorPosCallback(window.getWindow(), mouse_callback);
}
