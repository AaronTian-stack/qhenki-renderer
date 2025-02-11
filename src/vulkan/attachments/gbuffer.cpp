#include "gbuffer.h"

GBuffer::GBuffer(vk::Device device, vk::Extent2D resolution)
: FrameBuffer(device, vk::Framebuffer()), resolution(resolution)
{}

Attachment *GBuffer::getAttachment(GBufferAttachmentType type)
{
    if (type >= GBufferAttachmentType::END)
        throw std::runtime_error("Invalid GBufferAttachmentType");
    return attachmentMap[(unsigned int)type];
}

void GBuffer::setAttachment(GBufferAttachmentType type, uPtr<Attachment> &attachment, bool own)
{
    if (type >= GBufferAttachmentType::END)
        throw std::runtime_error("Invalid GBufferAttachmentType");
    attachmentMap[(unsigned int)type] = attachment.get();
    attachmentIndexMap[attachment.get()] = type;
    if (own)
        attachments.push_back(std::move(attachment));
}

void GBuffer::createFrameBuffer(vk::RenderPass renderPass)
{
    std::vector<vk::ImageView> imgAttachments;
    for (unsigned int i = 0; i < (unsigned int)GBufferAttachmentType::END; i++)
    {
        if (attachmentMap[i] != nullptr)
            imgAttachments.push_back(attachmentMap[i]->imageView);
        else
            throw std::runtime_error("GBuffer attachment not set");
    }
    vk::FramebufferCreateInfo createInfo(
            vk::FramebufferCreateFlags(),
            renderPass,
            imgAttachments.size(),
            imgAttachments.data(),
            resolution.width,
            resolution.height,
            1);
    this->framebuffer = device.createFramebuffer(createInfo);
}
