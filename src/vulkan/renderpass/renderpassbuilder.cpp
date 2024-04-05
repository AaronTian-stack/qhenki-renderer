#include "renderpassbuilder.h"

void RenderPassBuilder::destroy()
{}

void RenderPassBuilder::addColorAttachment(vk::Format format)
{
    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format = format;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
    addAttachment(&colorAttachment, vk::ImageLayout::eColorAttachmentOptimal);
}

void RenderPassBuilder::addDepthAttachment(vk::Format format)
{
    vk::AttachmentDescription depthAttachment{};
    depthAttachment.format = format;
    depthAttachment.samples = vk::SampleCountFlagBits::e1;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    addAttachment(&depthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

void RenderPassBuilder::addAttachment(vk::AttachmentDescription *attachment, vk::ImageLayout layout)
{
    vk::AttachmentReference attachmentRef{static_cast<uint32_t>(attachments.size()),layout};
    attachments.push_back(*attachment);
    attachmentRefs.push_back(attachmentRef);
}

void RenderPassBuilder::addSubPass(const std::vector<uint32_t> &indices, int depthIndex)
{
    // attachments subpass is writing to
    vk::SubpassDescription description{};
    refsVector.emplace_back(indices.size());
    auto &refs = refsVector.back();
    for (int i = 0; i < indices.size(); i++)
    {
        refs[i] = attachmentRefs[indices[i]];
        if (!subPasses.empty())
            refs[i].layout = vk::ImageLayout::eShaderReadOnlyOptimal;
    }
    description.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    description.colorAttachmentCount = refs.size();
    description.pColorAttachments = refs.data();
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
    refsVector.clear();
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
