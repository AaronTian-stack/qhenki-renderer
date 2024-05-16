#include "postprocessmanager.h"

PostProcessManager::PostProcessManager(vk::Device device, vk::Extent2D extent, BufferFactory &bufferFactory,
                                       RenderPassBuilder &renderPassBuilder)
: Destroyable(device), activeToneMapperIndex(0), currentAttachmentIndex(0)
{
    renderPassBuilder.reset();
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm);
    renderPassBuilder.addSubPass({}, {}, {0}, {});
    pingPongRenderPass = renderPassBuilder.buildRenderPass();

//    renderPassBuilder.reset();
//    renderPassBuilder.addColorAttachment(vk::Format::eR16G16B16A16Sfloat);
//    renderPassBuilder.addSubPass({}, {}, {0}, {});
//    toneMapRenderPass = renderPassBuilder.buildRenderPass();

    vk::Framebuffer fbs[2];
    for (int i = 0; i < 2; i++)
    {
        afb[i].attachment = bufferFactory.createAttachment(
                vk::Format::eR8G8B8A8Unorm,
                {extent.width, extent.height, 1},
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
                vk::ImageAspectFlagBits::eColor);
        vk::FramebufferCreateInfo createInfo(
                vk::FramebufferCreateFlags(),
                pingPongRenderPass->getRenderPass(),
                1,
                &afb[i].attachment->imageView,
                extent.width,
                extent.height,
                1);
        fbs[i] = device.createFramebuffer(createInfo);
    }
    for (int i = 0; i < 2; i++)
    {
        afb[i].framebuffer = fbs[1 - i];
    }
}

void PostProcessManager::tonemap(vk::CommandBuffer commandBuffer, DescriptorBuilder &builder, vk::DescriptorImageInfo *imageInfo)
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
    builder.bindImage(0, {*imageInfo},
                      1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .build(inputSet, layout);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, activeToneMapper->pipeline->getPipelineLayout(),
                                     0, {inputSet}, nullptr);

    activeToneMapper->bindData(commandBuffer);
    commandBuffer.draw(3, 1, 0, 0);
    pingPongRenderPass->end();
}

void PostProcessManager::render(vk::CommandBuffer commandBuffer, DescriptorBuilder &builder)
{
    int ping = 0; // start by reading from 0 and outputting to 1
    vk::DescriptorSetLayout layout;
    for (auto &postProcess : postProcesses)
    {
        pingPongRenderPass->setFramebuffer(afb[ping].framebuffer);
        auto extent = afb[ping].attachment->extent;
        pingPongRenderPass->setRenderAreaExtent({extent.width, extent.height});
        pingPongRenderPass->clear(0.f, 0.f, 0.f, 1.0f);
        pingPongRenderPass->begin(commandBuffer);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, postProcess->pipeline->getGraphicsPipeline());

        vk::DescriptorSet inputSet;
        auto descriptorInfo = afb[ping].attachment->getDescriptorInfo();
        builder.bindImage(0, {descriptorInfo},
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
    for (auto &postProcess : postProcesses)
    {
        postProcess->destroy();
    }
    for (auto &toneMapper : toneMappers)
    {
        toneMapper->destroy();
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

//RenderPass &PostProcessManager::getToneMapRenderPass()
//{
//    return *toneMapRenderPass;
//}
