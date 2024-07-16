#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <vulkan/vulkan_core.h>
#include "../window.h"
#include "../vulkan/renderpass/renderpass.h"
#include "../vulkan/queuemanager.h"
#include "../vulkan/commandpool.h"
#include "../vulkan/context/vulkancontext.h"
#include "../camera.h"
#include "../vfx/postprocessmanager.h"
#include "cameramenu.h"
#include "visualmenu.h"
#include "postprocessmenu.h"
#include "lightmenu.h"
#include <functional>

struct ImGuiCreateParameters
{
    VulkanContext *context;
    Window *window;
    RenderPass *renderPass;
    int framesInFlight;
};

struct MenuPayloads
{
    void *camera;
    void *postProcessManager;
    VisualMenuPayload visualMenuPayload;
    LightsList lightsList;
};

class UserInterface : Destroyable
{
private:
    Window *window;
    VkDescriptorPool imguiPool;
    void renderMenuBar();

    VisualMenu visualMenu;
    CameraMenu cameraMenu;
    PostProcessMenu postProcessMenu;
    LightMenu lightMenu;

    std::vector<float> frameTimes;

public:
    UserInterface();

    void create(ImGuiCreateParameters param, CommandPool &commandPool);
    void render(MenuPayloads menuPayloads);
    bool renderImage(vk::DescriptorSet image, ImVec2 size);
    void destroy() override;

    void begin();
    void end(VkCommandBuffer commandBuffer);

    std::function<void(std::string)> modelSelectCallback;

    friend class VulkanApp;
};

