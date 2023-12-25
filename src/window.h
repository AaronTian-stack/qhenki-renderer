#include "GLFW/glfw3.h"

class Window {
private:
    GLFWwindow* window;
    unsigned int SCR_WIDTH;
    unsigned int SCR_HEIGHT;

public:
    Window(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT);
    // TODO: needs to be added as callback
    void resize(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT);
    bool shouldClose();
    ~Window();
};
