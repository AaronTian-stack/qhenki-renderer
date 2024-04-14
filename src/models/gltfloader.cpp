// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include <iostream>
#include "gltfloader.h"
#include "glm/vec3.hpp"
#include <glm/gtx/matrix_decompose.hpp>

uPtr<Model> GLTFLoader::create(CommandPool &commandPool, QueueManager &queueManager, BufferFactory &bufferFactory, const char* filename)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model gltfModel;
    std::string err;
    std::string warn;

    loadStatus = LoadStatus::PARSING;
    bool ret = false;
    // check extension of filename
    std::string ext = filename;
    ext = ext.substr(ext.find_last_of('.') + 1);
    if (ext == "gltf")
        ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, filename);
    else if (ext == "glb")
        ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filename);
    else
        throw std::runtime_error("Invalid file extension");

    if (!warn.empty())
    {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret)
    {
        throw std::runtime_error("Failed to parse glTF");
    }

    uPtr<Model> model = mkU<Model>();

    int rootNodeIndex = gltfModel.scenes[gltfModel.defaultScene].nodes[0];

    loadStatus = LoadStatus::LOAD_MAT_TEX;
    makeMaterialsAndTextures(commandPool, queueManager, bufferFactory, gltfModel, model.get());
    loadStatus = LoadStatus::LOAD_NODE;
    processNode(bufferFactory, gltfModel, model.get(), nullptr, rootNodeIndex);

    std::cout << "FINISHED" << std::endl;

    return model;
}

void GLTFLoader::processNode(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model, Node *parent, int nodeIndex)
{
    const tinygltf::Node& gltfNode = gltfModel.nodes[nodeIndex];

    Node *node;
    if (!parent)
    {
        model->root = mkU<Node>();
        node = model->root.get();
    }
    else
    {
        parent->children.push_back(mkU<Node>());
        node = parent->children.back().get();
    }
    node->parent = parent;
    node->name = gltfNode.name;

    // set node transform
    if (gltfNode.scale.empty() && gltfNode.rotation.empty() && gltfNode.translation.empty() && gltfNode.matrix.size() == 16)
    {
        auto &m = gltfNode.matrix;
        glm::mat4 transformation = {
                m[0], m[1], m[2], m[3],
                m[4], m[5], m[6], m[7],
                m[8], m[9], m[10], m[11],
                m[12], m[13], m[14], m[15]
        };
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(transformation, scale, rotation, translation, skew, perspective);

        node->scale = scale;
        node->rotation = glm::conjugate(rotation);
        node->translate = translation;
    }
    else
    {
        if (!gltfNode.scale.empty())
        {
            auto &scale = gltfNode.scale;
            node->scale = {scale[0], scale[1], scale[2]};
        }
        if (!gltfNode.rotation.empty())
        {
            auto &rot = gltfNode.rotation;
            auto q = glm::dquat(rot[3], rot[0], rot[1], rot[2]);
            node->rotation = q;
        }
        if (!gltfNode.translation.empty())
        {
            auto &trans = gltfNode.translation;
            node->translate = {trans[0], trans[1], trans[2]};
        }
    }

    if (gltfNode.mesh >= 0)
    {
        const tinygltf::Mesh &gltfMesh = gltfModel.meshes[gltfNode.mesh];

        for (const auto &primitive : gltfMesh.primitives)
        {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
            {
                std::cout << "Not triangles\n";
                continue;
            }

            uPtr<Mesh> mesh = mkU<Mesh>();

            // extract vertex data
            for (auto &type : typeMap)
            {
                if (primitive.attributes.count(type.first) == 0)
                    continue;

                size_t vertexSize = type.second == VertexBufferType::UV ? sizeof(glm::vec2) : sizeof(glm::vec3);
                auto vBuffer = getBuffer(bufferFactory, gltfModel, primitive.attributes.at(type.first), vk::BufferUsageFlagBits::eVertexBuffer, vertexSize);
                mesh->vertexBuffers.emplace_back(std::move(vBuffer), type.second);
            }
            if (primitive.attributes.count("TANGENT") == 0)
            {
                std::cerr << "Tangent vectors manually generated!" << std::endl;
                // no tangent vectors, need to manually create them
                mesh->vertexBuffers.emplace_back(
                        createTangentVectors(bufferFactory, gltfModel,
                                             primitive.attributes.at("POSITION"),
                                             primitive.attributes.at("TEXCOORD_0"),
                                             vk::BufferUsageFlagBits::eVertexBuffer), VertexBufferType::TANGENT);
            }


            // extract index data
            mesh->indexBuffer = getBuffer(bufferFactory, gltfModel, primitive.indices, vk::BufferUsageFlagBits::eIndexBuffer, 0);

            mesh->material = &model->materials[primitive.material];

            model->meshes.push_back(std::move(mesh));
            node->mesh = model->meshes.back().get();
        }
    }

    // recursively process child nodes
    for (int childIndex : gltfNode.children)
    {
        processNode(bufferFactory, gltfModel, model, node, childIndex);
    }
}

uPtr<Buffer> GLTFLoader::getBuffer(BufferFactory &bufferFactory, tinygltf::Model &gltfModel,
                           int type, vk::BufferUsageFlagBits flag, size_t vertexSize)
{
    const tinygltf::Accessor &accessor = gltfModel.accessors[type];
    const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
    const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

    size_t count = accessor.count;
    uPtr<Buffer> vBuffer;

    switch (flag) {
        case vk::BufferUsageFlagBits::eVertexBuffer:
            vBuffer = bufferFactory.createBuffer(count * vertexSize, flag,
                                                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
            vBuffer->fill(&buffer.data[0] + bufferView.byteOffset + accessor.byteOffset);
            break;
        case vk::BufferUsageFlagBits::eIndexBuffer:
            // handle different formats
            if (accessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
            {
                // 16 bit
                vBuffer = bufferFactory.createBuffer(count * sizeof(uint16_t), flag,
                                                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
                vBuffer->fill(&buffer.data[0] + bufferView.byteOffset + accessor.byteOffset);
                vBuffer->setIndexType(vk::IndexType::eUint16);
            }
            else if (accessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT)
            {
                // 32 bit
                vBuffer = bufferFactory.createBuffer(count * sizeof(uint32_t), flag,
                                                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
                vBuffer->fill(&buffer.data[0] + bufferView.byteOffset + accessor.byteOffset);
                vBuffer->setIndexType(vk::IndexType::eUint32);
            }
            else
            {
                std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                throw std::runtime_error("Index type not supported");
            }
            break;
        default:
            throw std::runtime_error("Invalid buffer usage flag");
    }
    return vBuffer;
}

void GLTFLoader::makeMaterialsAndTextures(CommandPool &commandPool, QueueManager &queueManager, BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model)
{
    std::vector<vk::CommandBuffer> commandBuffers;
    std::vector<uPtr<Buffer>> stagingBuffers;
    // make every single image in order
    for (auto &image : gltfModel.images)
    {
        if (image.component != 4)
        {
            std::cerr << "Image component " << image.component << " not RGBA!" << std::endl;
            throw std::runtime_error("Image component not supported");
        }

        // gamma correction on color texture is done in the shader, otherwise load as unorm
        auto deferredImage = bufferFactory.createTextureImageDeferred(commandPool, vk::Format::eR8G8B8A8Unorm,
                                                             {static_cast<uint32_t>(image.width),
                                                              static_cast<uint32_t>(image.height), 1},
                                                             vk::ImageUsageFlagBits::eTransferDst |
                                                             vk::ImageUsageFlagBits::eSampled,
                                                             vk::ImageAspectFlagBits::eColor,
                                                             image.image.data());
        auto imageTexture = std::move(deferredImage.image);
        commandBuffers.push_back(deferredImage.cmd);
        stagingBuffers.push_back(std::move(deferredImage.stagingBufferToDestroy));
        model->images.push_back(std::move(imageTexture));
    }
    // submit as an async batch to make it smoother
    commandPool.submitSingleTimeCommands(queueManager, commandBuffers, vkb::QueueType::transfer, true);

    for (auto &buffer : stagingBuffers)
        buffer->destroy();

    for (int i = 0; i < gltfModel.textures.size(); i++)
    {
        const tinygltf::Texture &texture = gltfModel.textures[i];
        // a texture is an image and a sampler
        tinygltf::Sampler sampler;

        if (texture.sampler != -1)
        {
            sampler = gltfModel.samplers[texture.sampler];
        }
        else
        {
            sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
            sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
            sampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
            sampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;
        }

        vk::SamplerCreateInfo samplerInfo{};
        switch(sampler.magFilter)
        {
            case TINYGLTF_TEXTURE_FILTER_NEAREST:
                samplerInfo.magFilter = vk::Filter::eNearest;
                break;
            case TINYGLTF_TEXTURE_FILTER_LINEAR:
                samplerInfo.magFilter = vk::Filter::eLinear;
                break;
            default:
                std::cerr << "Mag filter " << sampler.magFilter << " not supported!" << std::endl;
                samplerInfo.magFilter = vk::Filter::eLinear;
                break;
        }
        switch(sampler.minFilter)
        {
            case TINYGLTF_TEXTURE_FILTER_NEAREST:
                samplerInfo.minFilter = vk::Filter::eNearest;
                break;
            case TINYGLTF_TEXTURE_FILTER_LINEAR:
                samplerInfo.minFilter = vk::Filter::eLinear;
                break;
            default:
                std::cerr << "Min filter " << sampler.minFilter << " not supported!" << std::endl;
                samplerInfo.minFilter = vk::Filter::eLinear;
                break;
        }
        switch(sampler.wrapS)
        {
            case TINYGLTF_TEXTURE_WRAP_REPEAT:
                samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
                break;
            case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
                break;
            default:
                std::cerr << "Wrap S " << sampler.wrapS << " not supported!" << std::endl;
                samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
                break;
        }
        switch(sampler.wrapT)
        {
            case TINYGLTF_TEXTURE_WRAP_REPEAT:
                samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
                break;
            case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
                break;
            default:
                std::cerr << "Wrap T " << sampler.wrapT << " not supported!" << std::endl;
                samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
                break;
        }
        model->textures.emplace_back(mkU<Texture>(model->images[texture.source].get()));
        model->textures.back()->createSampler(samplerInfo);
    }

    for (const auto &mat : gltfModel.materials)
    {
        Material material;
        material.baseColorFactor = glm::vec4(mat.pbrMetallicRoughness.baseColorFactor[0],
                                             mat.pbrMetallicRoughness.baseColorFactor[1],
                                             mat.pbrMetallicRoughness.baseColorFactor[2],
                                             mat.pbrMetallicRoughness.baseColorFactor[3]);
        material.metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
        material.roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;

        material.baseColorTexture = mat.pbrMetallicRoughness.baseColorTexture.index;
        material.metallicRoughnessTexture = mat.pbrMetallicRoughness.metallicRoughnessTexture.index;
        material.normalTexture = mat.normalTexture.index;

        material.occlusionTexture = mat.occlusionTexture.index;
        material.occlusionStrength = mat.occlusionTexture.strength;

        material.emissiveTexture = mat.emissiveTexture.index;

        model->materials.push_back(material);
    }
}

uPtr<Buffer> GLTFLoader::createTangentVectors(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, int verticesType,
                                              int uvType, vk::BufferUsageFlagBits flag)
{
    const tinygltf::Accessor &vAccessor = gltfModel.accessors[verticesType];
    const tinygltf::BufferView &vBufferView = gltfModel.bufferViews[vAccessor.bufferView];
    const tinygltf::Buffer &vBuffer = gltfModel.buffers[vBufferView.buffer];
    const auto* positions = reinterpret_cast<const glm::vec3*>(&vBuffer.data[vBufferView.byteOffset + vAccessor.byteOffset]);

    const tinygltf::Accessor &uvAccessor = gltfModel.accessors[uvType];
    const tinygltf::BufferView &uvBufferView = gltfModel.bufferViews[uvAccessor.bufferView];
    const tinygltf::Buffer &uvBuffer = gltfModel.buffers[uvBufferView.buffer];
    const auto* uvs = reinterpret_cast<const glm::vec2*>(&uvBuffer.data[uvBufferView.byteOffset + uvAccessor.byteOffset]);

    size_t count = vAccessor.count;
    if (vAccessor.count != uvAccessor.count)
    {
        std::cerr << "Vertex count " << vAccessor.count << " does not match UV count " << uvAccessor.count << std::endl;
    }

    auto buffer = bufferFactory.createBuffer(count * sizeof(glm::vec3), flag,
                                              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    std::vector<glm::vec3> tangents(count);
    for (size_t i = 0; i < count; ++i)
    {
        glm::vec3 v0 = positions[i+0];
        glm::vec3 v1 = positions[i+1];
        glm::vec3 v2 = positions[i+2];

        glm::vec2 uv0 = uvs[i+0];
        glm::vec2 uv1 = uvs[i+1];
        glm::vec2 uv2 = uvs[i+2];

        glm::vec3 deltaPos1 = v1 - v0;
        glm::vec3 deltaPos2 = v2 - v0;

        // UV delta
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
        tangents[i] = tangent;
    }

    buffer->fill(tangents.data());
    return buffer;
}

LoadStatus GLTFLoader::getLoadStatus()
{
    return loadStatus.load();
}

void GLTFLoader::setLoadStatus(LoadStatus status)
{
    loadStatus.store(status);
}
