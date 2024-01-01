#include <vulkan/vulkan.h>
#include <vector>
#include "vulkanpipeline.h"
#include "vulkanshader.h"
#include "vulkanrenderpass.h"
#include "../smartpointer.h"

class VkPipelineBuilder
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
    VkPipelineBuilder();
    uPtr<VulkanPipeline> buildPipeline(VkDevice device, VulkanRenderPass &renderPass, VulkanShader &shader);
    void reset();
};
