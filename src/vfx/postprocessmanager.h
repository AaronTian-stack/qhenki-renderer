#pragma once

#include "postprocess.h"
#include "../vulkan/attachments/attachment.h"
#include "../vulkan/buffer/bufferfactory.h"
#include "../vulkan/renderpass/renderpassbuilder.h"
#include "../vulkan/descriptors/descriptorbuilder.h"

struct AttachmentFrameBuffer
{
    uPtr<Attachment> attachment; // read from
    vk::Framebuffer framebuffer; // framebuffer to write to attachment
};

class PostProcessManager : public Destroyable
{
private:
    std::vector<sPtr<PostProcess>> toneMappers;
    int activeToneMapperIndex;
    std::vector<sPtr<PostProcess>> postProcesses;
    std::vector<PostProcess*> activePostProcesses;
    uPtr<RenderPass> pingPongRenderPass;
    std::array<AttachmentFrameBuffer, 2> afb;
    int currentAttachmentIndex;

public:
    PostProcessManager(vk::Device device, vk::Extent2D extent,
                       BufferFactory &bufferFactory, RenderPassBuilder &renderPassBuilder);
    void tonemap(vk::CommandBuffer commandBuffer, DescriptorBuilder &builder, vk::DescriptorImageInfo *imageInfo);
    void render(vk::CommandBuffer commandBuffer, DescriptorBuilder &builder);
    RenderPass& getPingPongRenderPass();
//    RenderPass& getToneMapRenderPass();
    void addToneMapper(const sPtr<PostProcess> &toneMapper);
    void addPostProcess(const sPtr<PostProcess> &postProcess);
    void destroy() override;
};
