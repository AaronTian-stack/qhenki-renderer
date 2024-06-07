#include "renderpassbuilder.h"

void RenderPassBuilder::destroy()
{}

void RenderPassBuilder::addColorAttachment(vk::Format format, vk::AttachmentLoadOp loadOp, vk::ImageLayout finalLayout)
{
    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format = format;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = loadOp;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = finalLayout;
    addAttachment(&colorAttachment, vk::ImageLayout::eColorAttachmentOptimal);
}

void RenderPassBuilder::addDepthAttachment(vk::Format format, vk::AttachmentLoadOp loadOp, vk::ImageLayout finalLayout)
{
    vk::AttachmentDescription depthAttachment{};
    depthAttachment.format = format;
    depthAttachment.samples = vk::SampleCountFlagBits::e1;
    depthAttachment.loadOp = loadOp;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
    if (loadOp == vk::AttachmentLoadOp::eLoad)
        depthAttachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    depthAttachment.finalLayout = finalLayout;
    addAttachment(&depthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

void RenderPassBuilder::addAttachment(vk::AttachmentDescription *attachment, vk::ImageLayout layout)
{
    vk::AttachmentReference attachmentRef{static_cast<uint32_t>(attachments.size()),layout};
    attachments.push_back(*attachment);
    attachmentRefs.push_back(attachmentRef);
}

void RenderPassBuilder::addSubPass(const std::vector<uint32_t> &inputIndices,
                                   const std::vector<vk::ImageLayout> &inputLayouts,
                                   const std::vector<uint32_t> &outputIndices,
                                   const std::vector<vk::ImageLayout> &outputLayouts,
                                   int depthIndex)
{
    // attachments subpass is writing to
    vk::SubpassDescription description{};
    outputRefsVector.emplace_back(outputIndices.size());
    auto &outputRefs = outputRefsVector.back();
    for (int i = 0; i < outputIndices.size(); i++)
    {
        outputRefs[i] = attachmentRefs[outputIndices[i]];
        if (!outputLayouts.empty())
            outputRefs[i].layout = outputLayouts[i % inputLayouts.size()];
    }
    description.colorAttachmentCount = outputRefs.size(); // this is output
    description.pColorAttachments = outputRefs.data();

    // attachments subpass is reading from
    inputRefsVector.emplace_back(inputIndices.size());
    auto &inputRefs = inputRefsVector.back();
    for (int i = 0; i < inputIndices.size(); i++)
    {
        inputRefs[i] = attachmentRefs[inputIndices[i]];
        if (!inputLayouts.empty())
            inputRefs[i].layout = inputLayouts[i % inputLayouts.size()];
    }
    description.inputAttachmentCount = inputRefs.size();
    description.pInputAttachments = inputRefs.data();

    description.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

    if (depthIndex != -1)
        description.pDepthStencilAttachment = &attachmentRefs[depthIndex];
    subPasses.push_back(description);
}

uPtr<RenderPass> RenderPassBuilder::buildRenderPass()
{
    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data(); // attachment array
    renderPassInfo.subpassCount = subPasses.size();
    renderPassInfo.pSubpasses = subPasses.data();
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();
    vk::RenderPass renderPass = device.createRenderPass(renderPassInfo);
    std::vector<vk::Format> formats(attachments.size());
    for (int i = 0; i < attachments.size(); i++)
    {
        formats[i] = attachments[i].format;
    }
    return mkU<RenderPass>(device, renderPass, formats);
}

void RenderPassBuilder::reset()
{
    attachments.clear();
    attachmentRefs.clear();
    subPasses.clear();
    inputRefsVector.clear();
    outputRefsVector.clear();
    dependencies.clear();
}

void RenderPassBuilder::addColorDependency(int srcSubpass, int dstSubpass)
{
    vk::SubpassDependency dependency{};
    dependency.srcSubpass = srcSubpass;
    dependency.dstSubpass = dstSubpass;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
    dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    dependency.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    dependency.dependencyFlags = vk::DependencyFlagBits::eByRegion;
    dependencies.push_back(dependency);
}

void RenderPassBuilder::addDepthDependency(int srcSubpass, int dstSubpass)
{
    throw std::runtime_error("not tested");
    vk::SubpassDependency dependency{};
    dependency.srcSubpass = srcSubpass;
    dependency.dstSubpass = dstSubpass;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eLateFragmentTests;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
    dependency.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    dependency.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    dependency.dependencyFlags = vk::DependencyFlagBits::eByRegion;
    dependencies.push_back(dependency);
}

