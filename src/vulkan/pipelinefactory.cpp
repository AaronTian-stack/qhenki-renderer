#include "pipelinefactory.h"

PipelineBuilder::PipelineBuilder() : pushOffset(0)
{
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    reset();
}

void PipelineBuilder::reset()
{
    vertexInputBindings.clear();
    vertexInputAttributes.clear();

    // NO VERTEX DATA!
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    // TODO: add option to change primitive type
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill; // TODO: add option to change polygon mode (need to enable feature)
    rasterizer.lineWidth = 1.0f; // TODO: add option to change line width (need to enable feature)
    // TODO: add option to change culling mode
    rasterizer.cullMode = vk::CullModeFlagBits::eFront;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;

    // built in bias maybe useful for shadow mapping
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // requires enabling feature
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // TODO: depth and stencil testing here

    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne; // Optional
    colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
    colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
    colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd; // Optional

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy; // Optional
    // number of color attachments
    colorBlending.attachmentCount = 1;
    // all the color attachments
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    //// PIPELINE LAYOUT ////

    pushConstants.clear();
    pushOffset = 0;

    // will be nothing and 0
    pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
    pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();

    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
}

uPtr<Pipeline> PipelineBuilder::buildPipeline(vk::Device device, RenderPass* renderPass, Shader* shader)
{
    auto pipeline = mkU<Pipeline>(device);

    vk::GraphicsPipelineCreateInfo pipelineInfo{};

    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shader->getShaderStages().data();

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = pipeline->createPipelineLayout(pipelineLayoutInfo);

    pipelineInfo.renderPass = renderPass->getRenderPass();
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    pipeline->createGraphicsPipeline(pipelineInfo);

    return pipeline;
}

void PipelineBuilder::addPushConstant(uint32_t size, vk::ShaderStageFlags stageFlags)
{
    // TODO: there seem to be issues if the stageFlags is anything besides VK_SHADER_STAGE_ALL
    vk::PushConstantRange pushConstant(
        stageFlags,
        pushOffset,
        size
            );

    pushConstants.push_back(pushConstant);

    pushOffset += size; // alignment is done in order

    pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
    pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();
}

void PipelineBuilder::addVertexInputBinding(vk::VertexInputBindingDescription binding)
{
    vertexInputBindings.emplace_back(binding);
    vertexInputInfo.vertexBindingDescriptionCount = vertexInputBindings.size();
    vertexInputInfo.pVertexBindingDescriptions = vertexInputBindings.data();
}

void PipelineBuilder::addVertexInputAttribute(vk::VertexInputAttributeDescription attribute)
{
    vertexInputAttributes.emplace_back(attribute);
    vertexInputInfo.vertexAttributeDescriptionCount = vertexInputAttributes.size();
    vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();
}
