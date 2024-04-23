#include "envmap.h"

#include <fstream>
#include <vector>
#include <iterator>

#define DDSKTX_IMPLEMENT
#include <dds/dds-ktx.h>

void EnvironmentMap::create(BufferFactory &bufferFactory, CommandPool &commandPool, QueueManager &queueManager, const char *path)
{
    // read the file
    std::ifstream file(path, std::ios::binary);
    std::vector<char> fileData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    ddsktx_texture_info tc = {0};
    ddsktx__parse_dds(&tc, fileData.data(), fileData.size(), NULL);

    // each outer vector is mip level
    std::vector<std::vector<ddsktx_sub_data>> faces;
    vk::DeviceSize size = 0;

    for (int i = 0; i < tc.num_mips; i++)
    {
        faces.emplace_back();
        // mip level
        auto &face = faces.back();
        // push back data
        for (int j = 0; j < 6; j++)
        {
            // The faces are written in the order: positive x, negative x, positive y, negative y, positive z, negative z DIRECTX
            // might need to swap the z +- directions
            ddsktx_sub_data sub_data;
            ddsktx_get_sub(&tc, &sub_data, fileData.data(), fileData.size(), 0, j, i);
            face.push_back(sub_data);
            size += sub_data.size_bytes;
        }
    }

    // cmft exports as BGR8... need to account for
    auto stagingBuffer = bufferFactory.createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    int offset = 0;
    for (int i = 0; i < tc.num_mips; i++)
    {
        auto &faceMipLeveli = faces[i];
        for (int j = 0; j < 6; j++)
        {
            auto &face = faceMipLeveli[j];
            stagingBuffer->fill(face.buff, offset, face.size_bytes);
            offset += face.size_bytes;
        }
    }

    vk::Format imageFormat;
    switch (tc.format)
    {
    case DDSKTX_FORMAT_RGBA16F: // should be radiance map
        imageFormat = vk::Format::eR16G16B16A16Sfloat;
        break;
    case DDSKTX_FORMAT_RGBA8: // should be irradiance map
        imageFormat = vk::Format::eR8G8B8A8Unorm;
        break;
    case DDSKTX_FORMAT_BGRA8: // should be irradiance map
        imageFormat = vk::Format::eB8G8R8A8Unorm;
        break;
    default:
        throw std::runtime_error("Unsupported format");
    }

    // create the image
    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.format = imageFormat;
    imageInfo.extent = vk::Extent3D(tc.width, tc.height, 1);
    imageInfo.mipLevels = tc.num_mips;
    imageInfo.arrayLayers = 6;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;

    vk::ImageViewCreateInfo viewInfo;
    viewInfo.viewType = vk::ImageViewType::eCube;
    viewInfo.format = imageFormat;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = tc.num_mips;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 6;

    this->maxMipLevels = tc.num_mips;

    cubeImage = mkU<Image>(bufferFactory.createAttachment(imageInfo, viewInfo, imageFormat));

    auto commandBuffer = commandPool.beginSingleCommand();

    // copy the data from staging buffer into cubemap
//    auto barrier = vk::ImageMemoryBarrier(
//            vk::AccessFlags(),
//            vk::AccessFlags(),
//            vk::ImageLayout::eUndefined,
//            vk::ImageLayout::eTransferDstOptimal,
//            VK_QUEUE_FAMILY_IGNORED,
//            VK_QUEUE_FAMILY_IGNORED,
//            cubeMap->attachment->image,
//            vk::ImageSubresourceRange(
//                    vk::ImageAspectFlagBits::eColor,
//                    0, tc.num_mips,
//                    0, 6)
//    );
//
//     commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);
    Image::recordTransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                                       cubeImage->attachment->image, commandBuffer, tc.num_mips, 6);

    std::vector<vk::BufferImageCopy> regions;

    vk::DeviceSize mapOffset = 0;
    for (int mipLevel = 0; mipLevel < tc.num_mips; mipLevel++)
    {
        for (int face = 0; face < 6; face++)
        {
            vk::BufferImageCopy region = {};
            region.bufferOffset = mapOffset;

            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = mipLevel;
            region.imageSubresource.baseArrayLayer = face;
            region.imageSubresource.layerCount = 1;
            region.imageExtent.width = tc.width >> mipLevel;
            region.imageExtent.height = tc.height >> mipLevel;
            region.imageExtent.depth = 1;

            mapOffset += faces[mipLevel][face].size_bytes;

            regions.push_back(region);
        }
    }

    commandBuffer.copyBufferToImage(stagingBuffer->buffer, cubeImage->attachment->image,
                                    vk::ImageLayout::eTransferDstOptimal, regions.size(), regions.data());

    Image::recordTransitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                                       cubeImage->attachment->image, commandBuffer, tc.num_mips, 6);

    commandBuffer.end();

    commandPool.submitSingleTimeCommands(queueManager, {commandBuffer}, vkb::QueueType::graphics, true);

    // make a sampler...
//    vk::SamplerCreateInfo samplerInfo{
//            vk::SamplerCreateFlags(),
//            vk::Filter::eLinear,
//            vk::Filter::eLinear,
//            vk::SamplerMipmapMode::eLinear,
//            vk::SamplerAddressMode::eClampToEdge,
//            vk::SamplerAddressMode::eClampToEdge,
//            vk::SamplerAddressMode::eClampToEdge,
//            0.0f,
//            VK_FALSE, // TODO: needs hardware support
//            16,
//            VK_FALSE,
//            vk::CompareOp::eAlways,
//            0.0f,
//            0.0f,
//            vk::BorderColor::eIntOpaqueBlack,
//            VK_FALSE
//    };
//    cubeMap->attachment->sampler = bufferFactory.device.createSampler(samplerInfo);

    cubeMap = mkU<Texture>(cubeImage.get());
    cubeMap->createSampler();

    // destroy the staging buffer
    stagingBuffer->destroy();
}

void EnvironmentMap::destroy()
{
    cubeMap->destroy();
}
