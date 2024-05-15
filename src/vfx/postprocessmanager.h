#pragma once

#include "postprocess.h"
#include "../vulkan/attachments/attachment.h"
#include "../vulkan/buffer/bufferfactory.h"
#include "../vulkan/renderpass/renderpassbuilder.h"

struct AttachmentFrameBuffer
{
    uPtr<Attachment> attachment; // read from
    vk::Framebuffer framebuffer; // output to
};

class PostProcessManager : public Destroyable
{
private:
    std::vector<uPtr<PostProcess>> postProcesses;
    std::vector<PostProcess*> activePostProcesses;
    uPtr<RenderPass> pingPongRenderPass;
    std::array<AttachmentFrameBuffer, 2> afb;
    Attachment *currentAttachment;

public:
    PostProcessManager(vk::Device device, vk::Extent2D extent,
                       BufferFactory &bufferFactory, RenderPassBuilder &renderPassBuilder);
    void render(vk::CommandBuffer commandBuffer);
    void destroy() override;
};
