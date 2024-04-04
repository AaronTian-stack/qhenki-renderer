#include <vulkan/vulkan.h>
#include "destroyable.h"
#include "syncer.h"
#include "commandpool.h"
#include "buffer/buffer.h"
#include "../smartpointer.h"
#include "buffer/bufferfactory.h"

class Frame : public Destroyable
{
private:
    vk::PipelineStageFlags waitStages[1] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

public:
    vk::CommandBuffer commandBuffer;
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;
    vk::Fence inFlightFence;

    Frame();
    Frame(vk::Device device, CommandPool &pool, Syncer &sync);

    void destroy() override;

    void begin();
    void end();

    vk::SubmitInfo getSubmitInfo();
    void finish(Syncer &syncer);
};
