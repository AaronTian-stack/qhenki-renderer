#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include "pipeline.h"
#include "shader.h"
#include "../renderpass/renderpass.h"
#include "../../smartpointer.h"
#include "../descriptors/descriptorlayoutcache.h"
#include <iostream>
#include "glm/glm.hpp"
#include "spirv_cross/spirv_glsl.hpp"

struct SetLayout
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    bool added;
};

struct BindInfo
{
    uint32_t binding;
    uint32_t set;
    uint32_t arrayLength;
};

class PipelineBuilder : public Destroyable
{
private:
    const std::vector<vk::DynamicState> dynamicStates =
    {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    BindInfo getBindInfo(const spirv_cross::CompilerGLSL &glsl, const spirv_cross::Resource &resource);

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    vk::PipelineViewportStateCreateInfo viewportState{};
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    vk::PipelineMultisampleStateCreateInfo multisampling{};
    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments; // you would make one of these for each attachment (specified in render pass)
    vk::PipelineColorBlendStateCreateInfo colorBlending{};

    std::array<SetLayout, 4> bindingsMap;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{}; // TODO: add way to change this

    uint32_t pushOffset;
    std::vector<vk::PushConstantRange> pushConstants;

    std::vector<vk::VertexInputBindingDescription> vertexInputBindings;
    std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;

public:
    PipelineBuilder();

    uPtr<Pipeline> buildPipeline(RenderPass* renderPass, int subpass, Shader* shader);
    void addPushConstant(uint32_t size, vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll);

    void processPushConstants(spirv_cross::CompilerGLSL &glsl, spirv_cross::ShaderResources &resources,
                              vk::ShaderStageFlags stages);

    void updateDescriptorSetLayouts(DescriptorLayoutCache &layoutCache);
    void parseVertexShader(const char *filePath, DescriptorLayoutCache &layoutCache, bool interleaved);

    void parseFragmentShader(const char *filePath, DescriptorLayoutCache &layoutCache);

    // buffer
    void addVertexInputBinding(vk::VertexInputBindingDescription binding);

    // attributes for buffer
    void addVertexInputAttribute(vk::VertexInputAttributeDescription attribute);

    void addDefaultColorBlendAttachment(int count);

    vk::PipelineRasterizationStateCreateInfo& getRasterizer();
    vk::PipelineColorBlendStateCreateInfo& getColorBlending();
    vk::PipelineDepthStencilStateCreateInfo& getDepthStencil();

    void reset();

    void destroy() override;

    void parseShader(const char *filePath1, const char *filePath2, DescriptorLayoutCache &layoutCache, bool interleaved);
};

std::pair<vk::Format, size_t> mapTypeToFormat(const spirv_cross::SPIRType &type);
