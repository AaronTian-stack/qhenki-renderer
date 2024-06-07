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

    // depth and stencil testing
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    colorBlendAttachments.clear();

    //// PIPELINE LAYOUT ////

    pushConstants.clear();
    pushOffset = 0;
    pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
    pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();

    bindingsMap.fill(SetLayout{});
    descriptorSetLayouts.clear();
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
}

uPtr<Pipeline> PipelineBuilder::buildPipeline(RenderPass* renderPass, int subpass, Shader* shader)
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
    pipelineInfo.pDepthStencilState = &depthStencil;

    addDefaultColorBlendAttachment(colorBlending.attachmentCount); // TODO: change this

    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = pipeline->createPipelineLayout(pipelineLayoutInfo);

    pipelineInfo.renderPass = renderPass->getRenderPass();
    pipelineInfo.subpass = subpass;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    pipeline->createGraphicsPipeline(pipelineInfo);

    return pipeline;
}

void PipelineBuilder::addPushConstant(uint32_t size, vk::ShaderStageFlags stageFlags)
{
    vk::PushConstantRange pushConstant(stageFlags,pushOffset,size);

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

void PipelineBuilder::processPushConstants(spirv_cross::CompilerGLSL &glsl, spirv_cross::ShaderResources &resources,
                                           vk::ShaderStageFlags stages)
{
    for (auto &pushConstant : resources.push_constant_buffers)
    {
        // get the layout information for the push constant block
        const spirv_cross::SPIRType& pushConstantType = glsl.get_type(pushConstant.type_id);

        std::cout << "Push Constant: " << pushConstant.name << std::endl;

        uint32_t size = 0;
        for (uint32_t i = 0; i < pushConstantType.member_types.size(); ++i)
        {
            size += glsl.get_declared_struct_member_size(pushConstantType, i);
        }
        addPushConstant(size, stages);
    }
}

void PipelineBuilder::updateDescriptorSetLayouts(DescriptorLayoutCache &layoutCache)
{
    // loop over all sets and create the descriptor set layouts
    for (auto &set : bindingsMap)
    {
        if (set.added || set.bindings.empty()) // already added to descriptor set layout
            continue;
        set.added = true;
        auto &list = set.bindings;
        vk::DescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.bindingCount = list.size(); // bindings in descriptor set
        layoutInfo.pBindings = list.data();

        descriptorSetLayouts.push_back(layoutCache.createDescriptorLayout(&layoutInfo));
    }

    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
}

void PipelineBuilder::parseVertexShader(const char *filePath, DescriptorLayoutCache &layoutCache, bool interleaved)
{
    auto shaderCode = Shader::readFile(filePath);
    auto spirvBytecode = reinterpret_cast<const uint32_t *>(shaderCode.data());

    spirv_cross::CompilerGLSL glsl(spirvBytecode, shaderCode.size() / sizeof(uint32_t));
    spirv_cross::ShaderResources resources = glsl.get_shader_resources();

    uint32_t binding = 0;
    uint32_t offset = 0;

    for (const auto &input : resources.stage_inputs)
    {
        std::cout << "Vertex Input Attribute: " << input.name << std::endl;

        // Get the layout information for the vertex input attribute
        const spirv_cross::SPIRType& inputType = glsl.get_type(input.type_id);

        uint32_t location = glsl.get_decoration(input.id, spv::DecorationLocation);

        auto format = mapTypeToFormat(inputType);
        std::cout << "Format Size: " << format.second << std::endl;

        addVertexInputAttribute({location, binding, format.first, offset});

        if (interleaved)
        {
            offset += format.second; // increment offset by format
        }
        else
        {
            binding++;
        }
    }

    processPushConstants(glsl, resources, vk::ShaderStageFlagBits::eVertex);

    //// CREATE DESCRIPTOR SET LAYOUT
    for (const auto &uniformBuffer : resources.uniform_buffers)
    {
        std::cout << "Uniform Buffer: " << uniformBuffer.name << std::endl;
        BindInfo bindInfo = getBindInfo(glsl, uniformBuffer);
        bindingsMap[bindInfo.set].bindings.emplace_back(bindInfo.binding,
                                            vk::DescriptorType::eUniformBuffer,
                                            bindInfo.arrayLength, // number of values in array
                                            vk::ShaderStageFlagBits::eVertex,
                                            nullptr // for images
                                            );
    }
    updateDescriptorSetLayouts(layoutCache);
}

void PipelineBuilder::parseFragmentShader(const char *filePath, DescriptorLayoutCache &layoutCache)
{
    auto shaderCode = Shader::readFile(filePath);
    auto spirvBytecode = reinterpret_cast<const uint32_t *>(shaderCode.data());

    spirv_cross::CompilerGLSL glsl(spirvBytecode, shaderCode.size() / sizeof(uint32_t));
    spirv_cross::ShaderResources resources = glsl.get_shader_resources();

    processPushConstants(glsl, resources, vk::ShaderStageFlagBits::eFragment);

    for (auto &sampledImage : resources.sampled_images)
    {
        std::cout << "Sampled Image: " << sampledImage.name << std::endl;
        BindInfo bindInfo = getBindInfo(glsl, sampledImage);
        bindingsMap[bindInfo.set].bindings.emplace_back(bindInfo.binding,
                                            vk::DescriptorType::eCombinedImageSampler,
                                            bindInfo.arrayLength, // number of values in array
                                            vk::ShaderStageFlagBits::eFragment,
                                            nullptr
                                            );
    }

    // get subpass inputs
    for (auto &subpassInput : resources.subpass_inputs)
    {
        std::cout << "Subpass Input: " << subpassInput.name << std::endl;
        BindInfo bindInfo = getBindInfo(glsl, subpassInput);
        bindingsMap[bindInfo.set].bindings.emplace_back(bindInfo.binding,
                                            vk::DescriptorType::eInputAttachment,
                                                        bindInfo.arrayLength, // number of values in array
                                            vk::ShaderStageFlagBits::eFragment,
                                            nullptr
                                            );
    }

    for (const auto &uniformBuffer : resources.uniform_buffers)
    {
        throw std::runtime_error("Uniform Buffers in Fragment Shader are not yet supported");
    }

    for (const auto &storageBuffer : resources.storage_buffers)
    {
        std::cout << "Storage Buffer" << storageBuffer.name << std::endl;
        BindInfo bindInfo = getBindInfo(glsl, storageBuffer);
        bindingsMap[bindInfo.set].bindings.emplace_back(bindInfo.binding,
                                            vk::DescriptorType::eStorageBuffer,
                                            bindInfo.arrayLength, // number of values in array
                                            vk::ShaderStageFlagBits::eFragment,
                                            nullptr
                                            );
    }

    updateDescriptorSetLayouts(layoutCache);
}

void PipelineBuilder::parseShader(const char *filePath1, const char *filePath2, DescriptorLayoutCache &layoutCache, bool interleaved)
{
    parseVertexShader(filePath1, layoutCache, interleaved);
    parseFragmentShader(filePath2, layoutCache);
}

std::pair<vk::Format, size_t> mapTypeToFormat(const spirv_cross::SPIRType &type)
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

vk::PipelineRasterizationStateCreateInfo& PipelineBuilder::getRasterizer()
{
    return rasterizer;
}

vk::PipelineColorBlendStateCreateInfo &PipelineBuilder::getColorBlending()
{
    return colorBlending;
}

vk::PipelineDepthStencilStateCreateInfo &PipelineBuilder::getDepthStencil()
{
    return depthStencil;
}

void PipelineBuilder::addDefaultColorBlendAttachment(int count)
{
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy; // Optional
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    for (int i = 0; i < count; i++)
    {
        vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne; // Optional
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd; // Optional
        colorBlendAttachments.push_back(colorBlendAttachment);
    }
    colorBlending.attachmentCount = colorBlendAttachments.size();
    colorBlending.pAttachments = colorBlendAttachments.data();
}

BindInfo PipelineBuilder::getBindInfo(const spirv_cross::CompilerGLSL &glsl, const spirv_cross::Resource &resource)
{
    uint32_t bindingNum = glsl.get_decoration(resource.id, spv::DecorationBinding);
    uint32_t set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
    uint32_t arrayLength = 1;
    auto sizes = glsl.get_type(resource.type_id).array;
    if (!sizes.empty()) // array
    {
        arrayLength = sizes[0];
    }
    return {bindingNum, set, arrayLength};
}
