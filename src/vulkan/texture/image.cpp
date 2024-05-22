#include "image.h"

Image::Image(const sPtr<Attachment> &attachment) : Destroyable(attachment->device), attachment(attachment), destroyed(false)
{}

void Image::recordTransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                        vk::Image image, vk::CommandBuffer commandBuffer, int mipCount, int layerCount)
{
    auto aspectMask = oldLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal ?
            vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;

    // TODO: mip levels
    auto barrier = vk::ImageMemoryBarrier(
            vk::AccessFlags(),
            vk::AccessFlags(),
            oldLayout,
            newLayout,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            image,
            vk::ImageSubresourceRange(aspectMask,
                                      0, mipCount,
                                      0, layerCount)
    );

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        // undefined -> transfer dst optimal
        barrier.srcAccessMask = vk::AccessFlags();
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
                newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        // transfer dst optimal -> shader read only optimal
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (oldLayout == vk::ImageLayout::eUndefined &&
                newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        // undefined -> shader read only optimal
        barrier.srcAccessMask = vk::AccessFlags();
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (oldLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal &&
                newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        // depth attachment -> shader read only optimal
        barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(
            sourceStage, destinationStage,
            vk::DependencyFlags(),
            nullptr, nullptr, barrier
    );
}

void Image::recordTransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                        vk::CommandBuffer commandBuffer)
{
    recordTransitionImageLayout(oldLayout, newLayout, attachment->image, commandBuffer, 1, 1);
}

void Image::destroy()
{
    attachment->destroy();
}

void Image::blit(vk::Image srcImage, vk::Image dstImage, vk::CommandBuffer commandBuffer, vk::ImageLayout srcLayout,
                 vk::ImageLayout dstLayout, vk::Extent3D extent)
{
    // TODO: replace with vk::ImageBlit2 for blitting multiple specific sub regions
    vk::ImageBlit blit(
            vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
            {vk::Offset3D(0, 0, 0), vk::Offset3D(extent.width, extent.height, extent.depth)},
            vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
            {vk::Offset3D(0, 0, 0), vk::Offset3D(extent.width, extent.height, extent.depth)}
    );
    commandBuffer.blitImage(
            srcImage, srcLayout,
            dstImage, dstLayout,
            1, &blit,
            vk::Filter::eLinear);

}

void Image::blit(Image &dstImage, vk::CommandBuffer commandBuffer)
{
    blit(attachment->image, dstImage.attachment->image, commandBuffer,
         vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eTransferDstOptimal, attachment->extent);
}
