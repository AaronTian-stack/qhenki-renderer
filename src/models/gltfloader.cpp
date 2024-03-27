// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include <iostream>
#include "gltfloader.h"
#include "glm/vec3.hpp"
#include <glm/gtx/matrix_decompose.hpp>

uPtr<Model> GLTFLoader::load(BufferFactory &bufferFactory, const char* filename)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model gltfModel;
    std::string err;
    std::string warn;

    bool ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filename); // for glb

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

    //makeTextures(bufferFactory, gltfModel, model.get());
    processNode(bufferFactory, gltfModel, model.get(), nullptr, rootNodeIndex);

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

            // extract index data
            mesh->indexBuffer = getBuffer(bufferFactory, gltfModel, primitive.indices, vk::BufferUsageFlagBits::eIndexBuffer, 0);

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

void GLTFLoader::makeTextures(CommandPool &commandPool, QueueManager &queueManager, BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model)
{
    // make every single image in order
    for (auto &image : gltfModel.images)
    {
        if (image.component != 4)
        {
            std::cerr << "Image component " << image.component << " not RGBA!" << std::endl;
            throw std::runtime_error("Image component not supported");
        }
        auto imageTexture = bufferFactory.createTextureImage(commandPool, queueManager, vk::Format::eR8G8B8A8Unorm,
                                                             {static_cast<uint32_t>(image.width),
                                                              static_cast<uint32_t>(image.height), 1},
                                                             vk::ImageUsageFlagBits::eTransferDst |
                                                             vk::ImageUsageFlagBits::eSampled,
                                                             vk::ImageAspectFlagBits::eColor,
                                                             image.image.data());
        model->images.push_back(std::move(imageTexture));
    }

    for (int i = 0; i < gltfModel.textures.size(); i++)
    {
        const tinygltf::Texture &texture = gltfModel.textures[i];
        // a texture is an image and a sampler
        const tinygltf::Sampler &sampler = gltfModel.samplers[texture.sampler];
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
        }
        model->textures.emplace_back(mkU<Texture>(model->images[texture.source].get()));
    }
}
