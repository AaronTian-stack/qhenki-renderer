#pragma once

#include "vulkan/vulkan_core.h"
#include "window.h"
#include "vulkan/renderpass/renderpass.h"
#include "vulkan/queuemanager.h"
#include "vulkan/commandpool.h"
#include "vulkan/context/vulkancontext.h"
#include "camera.h"

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
    bool optionsOpen = false;
    bool cameraOptionsOpen = false;

public:
    UserInterface();
    ~UserInterface();

    void create(ImGuiCreateParameters param, CommandPool commandPool);
    void render();
    void destroy() override;

    void begin();
    void end(VkCommandBuffer commandBuffer);

    std::vector<float> frameTimes;
    int currentShaderIndex = 0;
    float clearColor[3] = {0.0f, 0.0f, 0.0f };
};

