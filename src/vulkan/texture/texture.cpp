#include "texture.h"

Texture::Texture(std::unique_ptr<FrameBufferAttachment> attachment)
{
    this->attachment = std::move(attachment);
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

void Texture::destroy()
{
    attachment->destroy();
}
