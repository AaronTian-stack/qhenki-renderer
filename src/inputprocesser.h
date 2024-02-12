#pragma once

#include "glm/vec2.hpp"
#include "GLFW/glfw3.h"
#include "window.h"
#include <unordered_map>

class InputProcesser
{
private:
    inline static glm::vec2 lastMousePos;

public:
    static void create(Window &window);

    inline static glm::vec2 deltaMouse;
    inline static std::unordered_map<int, bool> justClickedMouseButtons;
    inline static std::unordered_map<int, bool> mouseButtons;

    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

    inline static float SENSITIVITY = 0.05f;
};
