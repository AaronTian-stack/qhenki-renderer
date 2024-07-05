#include "../postprocess.h"
#include "../../ui/menu.h"

class ChromaticAberration : public PostProcess
{
public:
    float maxIntensity;
    ChromaticAberration(vk::Device device, const char* shaderPath, PipelineBuilder &pipelineFactory,
              DescriptorLayoutCache &layoutCache, RenderPass *renderPass);
    void bindData(vk::CommandBuffer commandBuffer) override;
};
