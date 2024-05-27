#pragma once

#include "vulkan/vulkan_core.h"
#include "../window.h"
#include "../vulkan/renderpass/renderpass.h"
#include "../vulkan/queuemanager.h"
#include "../vulkan/commandpool.h"
#include "../vulkan/context/vulkancontext.h"
#include "../camera.h"
#include "../vfx/postprocessmanager.h"
#include <functional>

struct ImGuiCreateParameters
{
    VulkanContext *context;
    Window *window;
    RenderPass *renderPass;
    int framesInFlight;
};

class UserInterface : Destroyable
{
private:
    Window *window;
    VkDescriptorPool imguiPool;
    void renderMenuBar();
    void renderPostProcessStack(PostProcessManager *postProcessManager);
    bool optionsOpen = false;
    bool cameraOptionsOpen = false;
    bool drawBackground = true;
    bool postProcessOpen = false;

    float iblIntensity = 1.0f;
    float clearColor[3] = {0.25f, 0.25f, 0.25f};

public:
    UserInterface();
    ~UserInterface();

    void create(ImGuiCreateParameters param, CommandPool &commandPool);
    void render(void *postProcessManager);
    void destroy() override;

    void begin();
    void end(VkCommandBuffer commandBuffer);

    std::function<void(std::string)> modelSelectCallback;

    std::vector<float> frameTimes;

    friend class VulkanApp;
};

