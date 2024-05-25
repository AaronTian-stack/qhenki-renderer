#include "postprocessmanager.h"

PostProcessManager::PostProcessManager(vk::Device device, vk::Extent2D extent, BufferFactory &bufferFactory,
                                       RenderPassBuilder &renderPassBuilder)
: Destroyable(device), activeToneMapperIndex(0), currentAttachmentIndex(0)
{
    renderPassBuilder.reset();
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm);
    renderPassBuilder.addSubPass({}, {}, {0}, {});
    pingPongRenderPass = renderPassBuilder.buildRenderPass();

    for (int i = 0; i < 2; i++)
    {
        afb[i].attachment = bufferFactory.createAttachment(
                vk::Format::eR8G8B8A8Unorm,
                {extent.width, extent.height, 1},
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment
                | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc,
                vk::ImageAspectFlagBits::eColor);
        vk::FramebufferCreateInfo createInfo(
                vk::FramebufferCreateFlags(),
                pingPongRenderPass->getRenderPass(),
                1,
                &afb[i].attachment->imageView,
                extent.width,
                extent.height,
                1);
        afb[i].framebuffer = device.createFramebuffer(createInfo);
        afb[i].attachment->createGenericSampler(vk::Filter::eNearest, vk::SamplerMipmapMode::eLinear);
    }
}

void PostProcessManager::tonemap(vk::CommandBuffer commandBuffer,
                                 DescriptorLayoutCache &layoutCache, DescriptorAllocator &allocator,
                                 vk::DescriptorImageInfo *imageInfo)
{
    // tonemap into attachment 0
    pingPongRenderPass->setFramebuffer(afb[0].framebuffer);
    auto extent = afb[0].attachment->extent;
    pingPongRenderPass->setRenderAreaExtent({extent.width, extent.height});
    pingPongRenderPass->clear(0.f, 0.f, 0.f, 1.0f);
    pingPongRenderPass->begin(commandBuffer);

    auto &activeToneMapper = toneMappers[activeToneMapperIndex];
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, activeToneMapper->pipeline->getGraphicsPipeline());

    vk::DescriptorSetLayout layout;
    vk::DescriptorSet inputSet;
    DescriptorBuilder::beginSet(&layoutCache, &allocator).bindImage(0, {*imageInfo},
                      1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .build(inputSet, layout);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, activeToneMapper->pipeline->getPipelineLayout(),
                                     0, {inputSet}, nullptr);

    activeToneMapper->bindData(commandBuffer);
    commandBuffer.draw(3, 1, 0, 0);
    pingPongRenderPass->end();
}

void PostProcessManager::render(vk::CommandBuffer commandBuffer, DescriptorLayoutCache &layoutCache, DescriptorAllocator &allocator)
{
    int ping = 0; // start by reading from 0 and outputting to 1
    vk::DescriptorSetLayout layout;
    for (const auto &postProcess : activePostProcesses)
    {
        Image::recordTransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
                                           afb[ping].attachment->image, commandBuffer, 1, 1);

        pingPongRenderPass->setFramebuffer(afb[1 - ping].framebuffer);
        auto extent = afb[ping].attachment->extent;
        pingPongRenderPass->setRenderAreaExtent({extent.width, extent.height});
        pingPongRenderPass->clear(0.f, 0.f, 0.f, 1.0f);
        pingPongRenderPass->begin(commandBuffer);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, postProcess->pipeline->getGraphicsPipeline());

        vk::DescriptorSet inputSet;
        auto descriptorInfo = afb[ping].attachment->getDescriptorInfo();
        DescriptorBuilder::beginSet(&layoutCache, &allocator).bindImage(0, {descriptorInfo},
                           1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                .build(inputSet, layout);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, postProcess->pipeline->getPipelineLayout(),
                                                0, {inputSet}, nullptr);
        postProcess->bindData(commandBuffer);
        commandBuffer.draw(3, 1, 0, 0);
        ping = (ping + 1) % 2;

        pingPongRenderPass->end();
        currentAttachmentIndex = ping;
    }
}

void PostProcessManager::destroy()
{
    for (auto &attachment : afb)
    {
        attachment.attachment->destroy();
        device.destroyFramebuffer(attachment.framebuffer);
    }
    std::vector<PostProcess*> deleted;
    for (auto &postProcess : postProcesses)
    {
        if (std::find(deleted.begin(), deleted.end(), postProcess.get()) != deleted.end())
            continue;
        postProcess->destroy();
        deleted.push_back(postProcess.get());
    }
    for (auto &toneMapper : toneMappers)
    {
        if (std::find(deleted.begin(), deleted.end(), toneMapper.get()) != deleted.end())
            continue;
        toneMapper->destroy();
        deleted.push_back(toneMapper.get());
    }
    pingPongRenderPass->destroy();
}

void PostProcessManager::addToneMapper(const sPtr<PostProcess> &toneMapper)
{
    toneMappers.push_back(toneMapper);
}

void PostProcessManager::addPostProcess(const sPtr<PostProcess> &postProcess)
{
    postProcesses.push_back(postProcess);
}

RenderPass &PostProcessManager::getPingPongRenderPass()
{
    return *pingPongRenderPass;
}

Attachment* PostProcessManager::getCurrentAttachment()
{
    return afb[currentAttachmentIndex].attachment.get();
}

const std::vector<sPtr<PostProcess>> &PostProcessManager::getToneMappers()
{
    return toneMappers;
}

const std::vector<sPtr<PostProcess>> &PostProcessManager::getPostProcesses()
{
    return postProcesses;
}

const std::vector<PostProcess*> &PostProcessManager::getActivePostProcesses()
{
    return activePostProcesses;
}

const PostProcess *PostProcessManager::getActiveToneMapper()
{
    return toneMappers[activeToneMapperIndex].get();
}

void PostProcessManager::activatePostProcess(int index)
{
    activePostProcesses.push_back(postProcesses[index].get());
}

void PostProcessManager::deactivatePostProcess(int index)
{
    currentAttachmentIndex = 0;
    activePostProcesses.erase(activePostProcesses.begin() + index);
}
