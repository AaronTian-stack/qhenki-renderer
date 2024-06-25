#include "../postprocess.h"
#include "../../ui/menu.h"

class Sharpen : public PostProcess
{
public:
    float intensity;
    Sharpen(vk::Device device, const char* shaderPath, PipelineBuilder &pipelineFactory,
    DescriptorLayoutCache &layoutCache, RenderPass *renderPass);
    void bindData(vk::CommandBuffer commandBuffer) override;
};
