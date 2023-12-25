#include "application.h"
#include "window.h"
#include "smartpointer.h"
#include "vulkan/vkdebugger.h"

class PathTracerApp : public Application
{
private:
    uPtr<Window> window;
    VulkanInstance vulkanInstance;
    VKDebugger vulkanDebugger;

public:
    PathTracerApp();
    ~PathTracerApp();
    bool isAlive();
    void create() override;
    void render() override;
    void resize() override;
};
