#include "application.h"
#include "window.h"
#include "smartpointer.h"
#include "vulkan/vulkaninstance.h"

class PathTracerApp : public Application
{
private:
    uPtr<Window> window;
    VulkanInstance vulkanInstance;

public:
    PathTracerApp();
    ~PathTracerApp();
    bool isAlive();
    void create() override;
    void render() override;
    void resize() override;
};
