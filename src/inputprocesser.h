#pragma once

#include "glm/vec2.hpp"
#include "GLFW/glfw3.h"
#include "window.h"
#include <unordered_map>

class InputProcesser
{
private:
    inline static GLFWwindow *window;
    inline static glm::vec2 lastMousePos;

public:
    static void setCallbacks(Window &window);

    inline static glm::vec2 deltaMouse;
    inline static std::unordered_map<int, int> mouseButtons;

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    static void disableCursor();
    static void enableCursor();
    static int getCursorState();
    static void setUserPointer(void *ptr);

    inline static float SENSITIVITY_ROTATE = 0.05f;
    inline static float SENSITIVITY_TRANSLATE = 0.0025f;
};
