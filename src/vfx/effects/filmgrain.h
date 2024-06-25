#include "../postprocess.h"
#include "../../ui/menu.h"

struct FilmGrainData
{
    float seed;
    float intensity;
};

class FilmGrain : public PostProcess
{
public:
    FilmGrainData filmGrainData;
    FilmGrain(vk::Device device, const char* shaderPath, PipelineBuilder &pipelineFactory,
            DescriptorLayoutCache &layoutCache, RenderPass *renderPass);
    void bindData(vk::CommandBuffer commandBuffer) override;
    void updateTime(float time) override;
};
