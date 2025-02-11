#pragma once

#include <glm/vec2.hpp>
#include <GLFW/glfw3.h>
#include "window.h"
#include <unordered_map>
#include <tsl/robin_map.h>

class InputProcesser
{
private:
    inline static GLFWwindow *window;
    inline static glm::vec2 lastMousePos;
    inline static tsl::robin_map<int, int> buttons;

public:
    static void setCallbacks(Window &window);

    inline static glm::vec2 deltaMouse;

    static int getButton(int key);

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void disableCursor();
    static void enableCursor();
    static int getCursorState();
    static void setUserPointer(void *ptr);

    inline static float SENSITIVITY_ROTATE = 0.05f;
    inline static float SENSITIVITY_TRANSLATE = 0.01f;
    inline static float SENSITIVITY_ZOOM = 0.1f;
    inline static float SENSITIVITY_FOV = 5.0f;

    static void *getUserPointer();
};
