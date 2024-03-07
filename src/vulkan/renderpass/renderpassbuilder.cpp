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

void RenderPassBuilder::addAttachment(vk::AttachmentDescription *attachment, vk::ImageLayout layout)
{
    vk::AttachmentReference attachmentRef{
        static_cast<uint32_t>(attachments.size()),
        layout
    };
    attachments.push_back(*attachment);
    attachmentRefs.push_back(attachmentRef);
}

void RenderPassBuilder::addSubPass(const std::vector<uint32_t> &indices)
{
    // attachments subpass is writing to
    vk::SubpassDescription description{};
    refsVector.emplace_back(indices.size());
    auto &refs = refsVector.back();
    for (int i = 0; i < indices.size(); i++)
    {
        refs[i] = attachmentRefs[indices[i]];
    }
    description.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    description.colorAttachmentCount = refs.size();
    description.pColorAttachments = refs.data();
    subPasses.push_back(description);
}

uPtr<RenderPass> RenderPassBuilder::buildRenderPass()
{
    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data(); // attachment array
    renderPassInfo.subpassCount = subPasses.size();
    renderPassInfo.pSubpasses = subPasses.data();
    vk::RenderPass renderPass = device.createRenderPass(renderPassInfo);
    return mkU<RenderPass>(device, renderPass);
}

void RenderPassBuilder::reset()
{
    attachments.clear();
    attachmentRefs.clear();
    subPasses.clear();
    refsVector.clear();
}
