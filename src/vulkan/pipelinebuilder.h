#include <vulkan/vulkan.h>
#include <vector>
#include "pipeline.h"
#include "shader.h"
#include "renderpass.h"
#include "../smartpointer.h"

class PipelineBuilder
{
private:
    const std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineViewportStateCreateInfo viewportState{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineMultisampleStateCreateInfo multisampling{};
    VkPipelineDepthStencilStateCreateInfo depthStencil{}; // not used right now
    VkPipelineColorBlendAttachmentState colorBlendAttachment{}; // you would make one of these for each attachment (specified in render pass)
    VkPipelineColorBlendStateCreateInfo colorBlending{};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{}; // TODO: add way to change this

public:
    PipelineBuilder();
    uPtr<Pipeline> buildPipeline(VkDevice device, RenderPass* renderPass, Shader* shader);
    void reset();
};
