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
    rasterizer.cullMode = vk::CullModeFlagBits::eNone;
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
    pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
    pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();

    descriptorSetLayouts.clear();
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
}

uPtr<Pipeline> PipelineBuilder::buildPipeline(RenderPass* renderPass, Shader* shader)
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

void PipelineBuilder::parseVertexShader(const char *filePath, DescriptorLayoutCache &layoutCache)
{
    auto shaderCode = Shader::readFile(filePath);
    auto spirvBytecode = reinterpret_cast<const uint32_t *>(shaderCode.data());

    spirv_cross::CompilerGLSL glsl(spirvBytecode, shaderCode.size() / sizeof(uint32_t));
    spirv_cross::ShaderResources resources = glsl.get_shader_resources();

    uint32_t offset = 0;
    for (const auto &input : resources.stage_inputs)
    {
        std::cout << "Vertex Input Attribute: " << input.name << std::endl;

        // Get the layout information for the vertex input attribute
        spirv_cross::SPIRType inputType = glsl.get_type(input.type_id);

        uint32_t location = glsl.get_decoration(input.id, spv::DecorationLocation);

        auto format = mapTypeToFormat(inputType);

        addVertexInputAttribute({location, 0, format.first, offset});

        // increment offset by format
        offset += format.second;
    }

    for (auto &pushConstant : resources.push_constant_buffers)
    {
        // Get the layout information for the push constant block
        spirv_cross::SPIRType pushConstantType = glsl.get_type(pushConstant.type_id);

        std::cout << "Push Constant: " << pushConstant.name << std::endl;

        for (uint32_t i = 0; i < pushConstantType.member_types.size(); ++i)
        {
            uint32_t size = glsl.get_declared_struct_member_size(pushConstantType, i);

            addPushConstant(size, vk::ShaderStageFlagBits::eAll);
        }
    }

    //// CREATE DESCRIPTOR SET LAYOUT
    std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> bindingsMap;
    //std::vector<vk::DescriptorSetLayoutBinding> bindings;
    for (const auto &uniformBuffer : resources.uniform_buffers)
    {
        // the descriptors for the uniform buffers
        uint32_t bindingNum = glsl.get_decoration(uniformBuffer.id, spv::DecorationBinding);
        uint32_t set = glsl.get_decoration(uniformBuffer.id, spv::DecorationDescriptorSet); // TODO: needs to keep track of all sets!

        bindingsMap[set].emplace_back(bindingNum,
                           vk::DescriptorType::eUniformBuffer,
                           1, // number of values in array // TODO: get this via reflection
                           vk::ShaderStageFlagBits::eVertex,
                           nullptr // for images
                           );
    }
    // TODO: might want to do something eventually for storage buffers

    // loop over all sets and create the descriptor set layouts
    for (auto &set : bindingsMap)
    {
        vk::DescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.bindingCount = set.second.size(); // bindings in descriptor set
        layoutInfo.pBindings = set.second.data();
        descriptorSetLayouts.push_back(layoutCache.createDescriptorLayout(&layoutInfo));
    }

    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
}

void PipelineBuilder::parseFragmentShader(const char *filePath)
{
    auto shaderCode = Shader::readFile(filePath);
    auto spirvBytecode = reinterpret_cast<const uint32_t *>(shaderCode.data());

    spirv_cross::CompilerGLSL glsl(spirvBytecode, shaderCode.size() / sizeof(uint32_t));
    spirv_cross::ShaderResources resources = glsl.get_shader_resources();

    for (auto &pushConstant : resources.push_constant_buffers)
    {
        // Get the layout information for the push constant block
        spirv_cross::SPIRType pushConstantType = glsl.get_type(pushConstant.type_id);

        std::cout << "Push Constant: " << pushConstant.name << std::endl;

        for (uint32_t i = 0; i < pushConstantType.member_types.size(); ++i)
        {
            uint32_t offset = glsl.get_member_decoration(pushConstant.type_id, i, spv::DecorationOffset);
            uint32_t size = glsl.get_declared_struct_member_size(pushConstantType, i);

            addPushConstant(size, vk::ShaderStageFlagBits::eAll);
        }
    }
}

void PipelineBuilder::parseShader(const char *filePath1, const char *filePath2)
{
    parseFragmentShader(filePath1);
    parseFragmentShader(filePath2);
}

std::pair<vk::Format, size_t> PipelineBuilder::mapTypeToFormat(const spirv_cross::SPIRType &type)
{
    switch (type.basetype)
    {
        case spirv_cross::SPIRType::Float:
            switch (type.vecsize)
            {
                case 1:
                    return {vk::Format::eR32Sfloat, sizeof(float)};
                case 2:
                    return {vk::Format::eR32G32Sfloat, sizeof(glm::vec2)};
                case 3:
                    return {vk::Format::eR32G32B32Sfloat, sizeof(glm::vec3)};
                case 4:
                    return {vk::Format::eR32G32B32A32Sfloat, sizeof(glm::vec4)};
                default:
                    return {vk::Format::eUndefined, 0};
            }
        case spirv_cross::SPIRType::Int:
            switch(type.vecsize)
            {
                case 1:
                    return {vk::Format::eR32Sint, sizeof(int32_t)};
                case 2:
                    return {vk::Format::eR32G32Sint, sizeof(glm::ivec2)};
                case 3:
                    return {vk::Format::eR32G32B32Sint, sizeof(glm::ivec3)};
                case 4:
                    return {vk::Format::eR32G32B32A32Sint, sizeof(glm::ivec4)};
                default:
                    return {vk::Format::eUndefined, 0};
            }
        case spirv_cross::SPIRType::UInt:
            switch (type.vecsize)
            {
                case 1:
                    return {vk::Format::eR32Uint, sizeof(uint32_t)};
                case 2:
                    return {vk::Format::eR32G32Uint, sizeof(glm::uvec2)};
                case 3:
                    return {vk::Format::eR32G32B32Uint, sizeof(glm::uvec3)};
                case 4:
                    return {vk::Format::eR32G32B32A32Uint, sizeof(glm::uvec4)};
                default:
                    return {vk::Format::eUndefined, 0};
            }
        default:
            return {vk::Format::eUndefined, 0};
    }
}

void PipelineBuilder::destroy()
{}
