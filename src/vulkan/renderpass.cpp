#include "renderpass.h"
#include <stdexcept>

RenderPass::RenderPass()
{
    reset();
}

void RenderPass::create(VkDevice device)
{
    deviceForDispose = device;
    if (vkCreateRenderPass(deviceForDispose, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderPass::dispose()
{
    vkDestroyRenderPass(deviceForDispose, renderPass, nullptr);
}

void RenderPass::reset()
{
    colorAttachment.format = VK_FORMAT_A8B8G8R8_UINT_PACK32; // need to provide swapchain format
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    colorAttachmentRef.attachment = 0; // index in pAttachments array
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // TODO: abstract attachment setup
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
}

void RenderPass::setColorAttachmentFormat(VkFormat format)
{
    colorAttachment.format = format;
}

void RenderPass::begin(VkCommandBuffer commandBuffer)
{
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset = {0, 0};

    // also need renderPassInfo.framebuffer and renderPassInfo.renderArea.extent

    // set up the command buffer for this render pass
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    currentCommandBuffer = commandBuffer;
}

void RenderPass::clear(float r, float g, float b, float a)
{
    clearColor = {{{r, g, b, a}}};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
}

void RenderPass::setFramebuffer(VkFramebuffer buffer)
{
    renderPassBeginInfo.framebuffer = buffer;
}

void RenderPass::setRenderAreaExtent(VkExtent2D extent)
{
    renderPassBeginInfo.renderArea.extent = extent;
}

void RenderPass::end()
{
    vkCmdEndRenderPass(currentCommandBuffer);
    currentCommandBuffer = VK_NULL_HANDLE;
}
