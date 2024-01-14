#include <vulkan/vulkan.h>
#include <vector>
#include "pipeline.h"
#include "shader.h"
#include "renderpass.h"
#include "../smartpointer.h"

class PipelineBuilder
{
private:
    const std::vector<vk::DynamicState> dynamicStates =
    {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    vk::PipelineViewportStateCreateInfo viewportState{};
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    vk::PipelineMultisampleStateCreateInfo multisampling{};
    vk::PipelineDepthStencilStateCreateInfo depthStencil{}; // not used right now
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{}; // you would make one of these for each attachment (specified in render pass)
    vk::PipelineColorBlendStateCreateInfo colorBlending{};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{}; // TODO: add way to change this

    uint32_t pushOffset;
    std::vector<vk::PushConstantRange> pushConstants;

    std::vector<vk::VertexInputBindingDescription> vertexInputBindings;
    std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;

public:
    PipelineBuilder();
    uPtr<Pipeline> buildPipeline(vk::Device device, RenderPass* renderPass, Shader* shader);
    void addPushConstant(uint32_t size, vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll);

    // buffer
    void addVertexInputBinding(vk::VertexInputBindingDescription binding);

    // attributes for buffer
    void addVertexInputAttribute(vk::VertexInputAttributeDescription attribute);

    void reset();
};
