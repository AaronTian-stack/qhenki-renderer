#pragma once

#include "../destroyable.h"
#include "renderpass.h"
#include "../../smartpointer.h"

class RenderPassBuilder : public Destroyable
{
private:
    std::vector<vk::AttachmentDescription> attachments;
    std::vector<vk::AttachmentReference> attachmentRefs;

    std::vector<std::vector<vk::AttachmentReference>> inputRefsVector;
    std::vector<std::vector<vk::AttachmentReference>> outputRefsVector;
    std::vector<vk::SubpassDescription> subPasses;
    std::vector<vk::SubpassDependency> dependencies;

    void addAttachment(vk::AttachmentDescription *attachment, vk::ImageLayout layout);

public:

    void addColorAttachment(vk::Format format, vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear);
    void addDepthAttachment(vk::Format format, vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear);

    void addSubPass(const std::vector<uint32_t> &inputIndices,
                    const std::vector<vk::ImageLayout> &inputLayouts,
                    const std::vector<uint32_t> &outputIndices,
                    const std::vector<vk::ImageLayout> &outputLayouts,
                    int depthIndex = -1);
    void addColorDependency(int srcSubpass, int dstSubpass);
    void addDepthDependency(int srcSubpass, int dstSubpass);

    uPtr<RenderPass> buildRenderPass();
    void reset();
    void destroy() override;
};
