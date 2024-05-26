#include "../postprocess.h"
#include "../../ui/menu.h"

class Sharpen : public PostProcess, public Menu
{
private:
    float intensity;

public:
    Sharpen(vk::Device device, const char* shaderPath, PipelineBuilder &pipelineFactory,
    DescriptorLayoutCache &layoutCache, RenderPass *renderPass);
    void bindData(vk::CommandBuffer commandBuffer) override;
    void renderMenu() override;
};
