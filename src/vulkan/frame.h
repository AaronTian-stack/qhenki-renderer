#include <vulkan/vulkan.h>
#include "../disposable.h"
#include "syncer.h"
#include "commandpool.h"

class Frame : public Disposable
{
private:
    VkCommandBufferBeginInfo beginInfo{};
    VkSubmitInfo submitInfo{};

    VkPipelineStageFlags waitStages[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

public:

    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    Frame();
    Frame(VkDevice device, CommandPool &pool, Syncer &sync);
    void create(VkDevice device, CommandPool &pool, Syncer &sync);
    void dispose() override;

    void begin();
    void end();

    VkSubmitInfo getSubmitInfo();
    void finish(Syncer &syncer);
};
