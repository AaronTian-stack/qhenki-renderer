#include "texture.h"

Texture::Texture(std::unique_ptr<FrameBufferAttachment> attachment)
{
    this->attachment = std::move(attachment);
}

void Texture::createSampler()
{
    // TODO: add options
    vk::SamplerCreateInfo samplerInfo{
            vk::SamplerCreateFlags(),
            vk::Filter::eLinear,
            vk::Filter::eLinear,
            vk::SamplerMipmapMode::eLinear,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            0.0f,
            VK_FALSE, // TODO: needs hardware support
            16,
            VK_FALSE,
            vk::CompareOp::eAlways,
            0.0f,
            0.0f,
            vk::BorderColor::eIntOpaqueBlack,
            VK_FALSE
    };
    sampler = device.createSampler(samplerInfo);
}

void Texture::transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                    CommandPool &commandPool, QueueManager &queueManager)
{
    auto commandBuffer = commandPool.beginSingleCommand();

    // TODO: mip levels
    auto barrier = vk::ImageMemoryBarrier(
            vk::AccessFlags(),
            vk::AccessFlags(),
            oldLayout,
            newLayout,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            attachment->image,
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
    );

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        // undefined -> transfer dst optimal
        barrier.srcAccessMask = vk::AccessFlags();
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
             newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        // transfer dst optimal -> shader read only optimal
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(
            sourceStage, destinationStage,
            vk::DependencyFlags(),
            nullptr, nullptr, barrier
    );
    commandBuffer.end();

    commandPool.submitSingleTimeCommands(queueManager, {commandBuffer});
}

vk::DescriptorImageInfo Texture::getDescriptorInfo()
{
    vk::DescriptorImageInfo info{};
    info.imageView = attachment->imageView;
    info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    info.sampler = sampler;
    return info;
}

void Texture::destroy()
{
    attachment->destroy();
    if (sampler != nullptr)
        device.destroySampler(sampler);
}
