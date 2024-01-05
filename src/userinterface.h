#pragma once

#include "disposable.h"
#include "vulkan/vulkan_core.h"
#include "window.h"
#include "vulkan/devicepicker.h"
#include "vulkan/renderpass.h"
#include "vulkan/queuemanager.h"
#include "vulkan/commandpool.h"

struct ImGuiCreateParameters
{
    VulkanInstance *instance;
    DevicePicker *devicePicker;
    Window *window;
    RenderPass *renderPass;
    QueueManager *queueManager;
    int framesInFlight;
};

class UserInterface : Disposable
{
private:
    VkDescriptorPool imguiPool;
    void renderMenuBar();

public:
    UserInterface();

    void create(ImGuiCreateParameters param, CommandPool commandPool);
    void render();
    void dispose() override;

    void begin();
    void end(VkCommandBuffer commandBuffer);
};

