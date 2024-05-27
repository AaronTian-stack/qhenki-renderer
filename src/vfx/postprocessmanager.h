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
    std::vector<uPtr<PostProcess>> toneMappers;
    int activeToneMapperIndex;
    std::vector<sPtr<PostProcess>> postProcesses;
    std::vector<PostProcess*> activePostProcesses;
    uPtr<RenderPass> pingPongRenderPass;
    std::array<AttachmentFrameBuffer, 2> afb;
    int currentAttachmentIndex;

public:
    PostProcessManager(vk::Device device, vk::Extent2D extent,
                       BufferFactory &bufferFactory, RenderPassBuilder &renderPassBuilder);
    void tonemap(vk::CommandBuffer commandBuffer,
                 DescriptorLayoutCache &layoutCache, DescriptorAllocator &allocator,
                 vk::DescriptorImageInfo *imageInfo);
    void render(vk::CommandBuffer commandBuffer, DescriptorLayoutCache &layoutCache, DescriptorAllocator &allocator);
    RenderPass& getPingPongRenderPass();
    void addToneMapper(uPtr<PostProcess> &toneMapper);
    void addPostProcess(const sPtr<PostProcess> &postProcess);
    void setToneMapper(int index);
    void activatePostProcess(int index);
    void deactivatePostProcess(int index);
    Attachment* getAttachment(int index);
    Attachment* getCurrentAttachment();
    vk::Framebuffer getFramebuffer(int index);
    const std::vector<uPtr<PostProcess>>& getToneMappers();
    const std::vector<sPtr<PostProcess>>& getPostProcesses();
    const PostProcess* getActiveToneMapper();
    const std::vector<PostProcess*>& getActivePostProcesses();
    void destroy() override;
};
