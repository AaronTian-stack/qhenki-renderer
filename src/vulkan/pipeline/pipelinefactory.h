#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include "pipeline.h"
#include "shader.h"
#include "../renderpass/renderpass.h"
#include "../../smartpointer.h"
#include "spirv_cross/spirv_glsl.hpp"
#include "../descriptors/descriptorlayoutcache.h"
#include <iostream>
#include "glm/glm.hpp"

class PipelineBuilder : public Destroyable
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

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{}; // TODO: add way to change this

    uint32_t pushOffset;
    std::vector<vk::PushConstantRange> pushConstants;

    std::vector<vk::VertexInputBindingDescription> vertexInputBindings;
    std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;

    std::pair<vk::Format, size_t> mapTypeToFormat(const spirv_cross::SPIRType &type);

public:
    PipelineBuilder();

    uPtr<Pipeline> buildPipeline(RenderPass* renderPass, Shader* shader);
    void addPushConstant(uint32_t size, vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll);

    void parseShader(const char *filePath1, const char *filePath2);

    void parseVertexShader(const char *filePath, DescriptorLayoutCache &layoutCache);
    void parseFragmentShader(const char *filePath);

    // buffer
    void addVertexInputBinding(vk::VertexInputBindingDescription binding);

    // attributes for buffer
    void addVertexInputAttribute(vk::VertexInputAttributeDescription attribute);

    void reset();

    void destroy() override;
};
