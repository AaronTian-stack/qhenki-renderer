#include "envmap.h"
#include <gltf/stb_image.h>

#include <fstream>
#include <vector>
#include <iterator>

#define DDSKTX_IMPLEMENT
#include <dds/dds-ktx.h>
#include <thread>
#include <iostream>

#define MULTITHREAD_ENVIRONMENT_MAP

void EnvironmentMap::create(BufferFactory &bufferFactory, CommandPool &commandPool, QueueManager &queueManager,
                            const char *path)
{
    std::string sPath = std::string(path);
    auto rawPath = sPath.substr(0, sPath.find_last_of('.'));

    std::string irradiance = rawPath + std::string("_irradiance.dds");
    std::string radiance = rawPath + std::string("_radiance.dds");

    std::vector<vk::CommandBuffer> commandBuffers(4);
    auto cubeMapL = [&](vk::CommandBuffer *commandBuffer) {
        cubeMap = createCubeMap(commandBuffer, bufferFactory, commandPool, path);
    };
    auto irradianceMapL = [&](vk::CommandBuffer *commandBuffer) {
        irradianceMap = createCubeMap(commandBuffer, bufferFactory, commandPool, irradiance.c_str());
    };
    auto radianceMapL = [&](vk::CommandBuffer *commandBuffer) {
        radianceMap = createCubeMap(commandBuffer, bufferFactory, commandPool, radiance.c_str());
    };

    uPtr<Buffer> deferStagingBuffer;
    auto brdfLUTL = [&]() {
        //// BRDF LUT
        std::ifstream file("../resources/envmaps/brdf_lut.dds", std::ios::binary);
        std::vector<char> fileData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        ddsktx_texture_info tc = {0};
        ddsktx__parse_dds(&tc, fileData.data(), fileData.size(), NULL);
        ddsktx_sub_data sub_data;
        ddsktx_get_sub(&tc, &sub_data, fileData.data(), fileData.size(), 0, 0, 0);
        if (tc.format != DDSKTX_FORMAT_RG16F)
            throw std::runtime_error("brdf_lut format");

        auto defer = bufferFactory.createTextureImageDeferred(
                commandPool, vk::Format::eR16G16Sfloat,
                {static_cast<uint32_t>(tc.width),static_cast<uint32_t>(tc.height), 1},
                vk::ImageUsageFlagBits::eTransferDst |
                vk::ImageUsageFlagBits::eSampled,
                vk::ImageAspectFlagBits::eColor,
                const_cast<void *>(sub_data.buff));
        brdfLUT.image = std::move(defer.image);
        brdfLUT.texture = mkU<Texture>(brdfLUT.image.get());
        vk::SamplerCreateInfo samplerInfo{
                vk::SamplerCreateFlags(),
                vk::Filter::eLinear,
                vk::Filter::eLinear,
                vk::SamplerMipmapMode::eLinear,
                vk::SamplerAddressMode::eClampToEdge,
                vk::SamplerAddressMode::eClampToEdge,
                vk::SamplerAddressMode::eClampToEdge,
                0.0f,
                VK_FALSE,
                16,
                VK_FALSE,
                vk::CompareOp::eAlways,
                0.0f,
                1000.0f,
                vk::BorderColor::eIntOpaqueBlack,
                VK_FALSE
        };
        brdfLUT.texture->createSampler(samplerInfo);

        commandBuffers[3] = defer.cmd;
        deferStagingBuffer = std::move(defer.stagingBufferToDestroy);
    };

    auto time1 = std::chrono::high_resolution_clock::now();
#ifdef MULTITHREAD_ENVIRONMENT_MAP
    std::thread t1(cubeMapL, &commandBuffers[0]);
    std::thread t2(irradianceMapL, &commandBuffers[1]);
    std::thread t3(radianceMapL, &commandBuffers[2]);
    std::thread t4(brdfLUTL);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
#else
    cubeMapL(&commandBuffers[0]);
    irradianceMapL(&commandBuffers[1]);
    radianceMapL(&commandBuffers[2]);
    brdfLUTL();
#endif

    auto time2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time1).count();

    // ~0.5s faster multithreaded with metro_noord envmap
    std::cout << "Environment map creation time: " << duration << "ms" << std::endl;

    commandPool.submitSingleTimeCommands(queueManager, commandBuffers, true);

    for (auto &stagingBuffer : stagingBuffers)
    {
        stagingBuffer->destroy();
    }
    deferStagingBuffer->destroy();

    stagingBuffers.clear();
}

ImageTexture
EnvironmentMap::createCubeMap(vk::CommandBuffer *commandBuffer, BufferFactory &bufferFactory, CommandPool &commandPool,
                              const char *path)
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

    // IMPORTANT: cmft exports as BGR8
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
    default:
        throw std::runtime_error("Unsupported format, environment maps should all be 16f");
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

    auto cubeImage = mkU<Image>(bufferFactory.createAttachment(imageInfo, viewInfo, imageFormat));

    *commandBuffer = commandPool.beginSingleCommand();

    Image::recordTransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                                       cubeImage->attachment->image, *commandBuffer, tc.num_mips, 6);

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

    commandBuffer->copyBufferToImage(stagingBuffer->buffer, cubeImage->attachment->image,
                                    vk::ImageLayout::eTransferDstOptimal, regions.size(), regions.data());

    Image::recordTransitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                                       cubeImage->attachment->image, *commandBuffer, tc.num_mips, 6);

    commandBuffer->end();

    auto cubeMap = mkU<Texture>(cubeImage.get());
    cubeMap->createSampler();

    stagingBuffers.push_back(std::move(stagingBuffer));

    return {std::move(cubeImage), std::move(cubeMap)};
}

void EnvironmentMap::destroy()
{
    cubeMap.texture->destroy();
    irradianceMap.texture->destroy();
    radianceMap.texture->destroy();
    brdfLUT.texture->destroy();
}
