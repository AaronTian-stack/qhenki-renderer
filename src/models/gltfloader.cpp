#include "tangentcalc.h"

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include <iostream>
#include "gltfloader.h"
#include "glm/vec3.hpp"
#include <glm/gtx/matrix_decompose.hpp>
#include <thread>

uPtr<Model> GLTFLoader::create(CommandPool &commandPool, QueueManager &queueManager, BufferFactory &bufferFactory,
                               const char* filename, int framesInFlight)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model gltfModel;
    std::string err;
    std::string warn;

    loadPercent = 0.f;
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

    // TODO: multiple roots
    int rootNodeIndex = gltfModel.scenes[gltfModel.defaultScene].nodes[0];

    std::thread matThread([&](){
        makeMaterialsAndTextures(commandPool, queueManager, bufferFactory, gltfModel, model.get());
    });

    tsl::robin_map<Mesh*, int> meshMaterialMap;
    tsl::robin_map<int, Node*> numberNodeMap;
    std::thread nodeThread([&](){
       processNode(bufferFactory, gltfModel, model.get(), nullptr, rootNodeIndex, meshMaterialMap, numberNodeMap);
    });

    nodeThread.join();

    // assign joints to nodes
    processSkinsAnimations(bufferFactory, gltfModel, model.get(), numberNodeMap, framesInFlight);

    matThread.join();

    for (auto &mesh : model->meshes)
    {
        mesh->material = &model->materials[meshMaterialMap[mesh.get()]];
    }

    return model;
}

void GLTFLoader::processNode(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model, Node *parent, int nodeIndex,
                             tsl::robin_map<Mesh*, int> &meshMap, tsl::robin_map<int, Node*> &numberNodeMap)
{
    const tinygltf::Node& gltfNode = gltfModel.nodes[nodeIndex];
    loadPercent.store(loadPercent.load() + 0.5f / gltfModel.nodes.size());

    Node *node;
    if (!parent)
    {
        model->root = mkU<Node>(model);
        node = model->root.get();
    }
    else
    {
        parent->children.push_back(mkU<Node>(model));
        node = parent->children.back().get();
    }
    node->parent = parent;
    node->name = gltfNode.name;
    node->skinIndex = gltfNode.skin;

    numberNodeMap[nodeIndex] = node;

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

        node->transform.scale = scale;
        node->transform.rotation = glm::conjugate(rotation);
        node->transform.translate = translation;
    }
    else
    {
        if (!gltfNode.scale.empty())
        {
            auto &scale = gltfNode.scale;
            node->transform.scale = {scale[0], scale[1], scale[2]};
        }
        if (!gltfNode.rotation.empty())
        {
            auto &rot = gltfNode.rotation;
            auto q = glm::dquat(rot[3], rot[0], rot[1], rot[2]);
            node->transform.rotation = q;
        }
        if (!gltfNode.translation.empty())
        {
            auto &trans = gltfNode.translation;
            node->transform.translate = {trans[0], trans[1], trans[2]};
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

            auto flags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer;
            // extract vertex data
            for (auto &type : typeMap)
            {
                if (primitive.attributes.count(type.first) == 0)
                    continue;

                size_t vertexSize = (type.second == VertexBufferType::UV_0 || type.second == VertexBufferType::UV_1)
                        ? sizeof(glm::vec2) : sizeof(glm::vec3);
                mesh->vertexBuffers[type.second] = getBuffer(bufferFactory, gltfModel, primitive.attributes.at(type.first), flags);
            }
            if (primitive.attributes.count(TANGENT_STRING) == 0)
            {
                std::cerr << "Tangent vectors manually generated using MikkTSpace!" << std::endl;
                // no tangent vectors, need to manually create them
                mesh->vertexBuffers[VertexBufferType::TANGENT] =
                        createTangentVectors(bufferFactory, gltfModel,
                                             primitive.attributes.at(POSITION_STRING),
                                             primitive.attributes.at(NORMAL_STRING),
                                             primitive.attributes.at(TEXCOORD_STRING_0),
                                             primitive.indices,
                                             flags);
            }
            auto hasJoints = primitive.attributes.count(JOINTS_STRING) != 0;
            if (hasJoints)
            {
                mesh->jointsBuffer = getBuffer(bufferFactory, gltfModel, primitive.attributes.at(JOINTS_STRING), flags);
            }
            auto hasWeights = primitive.attributes.count(WEIGHTS_STRING) != 0;
            if (hasWeights)
            {
                mesh->weightsBuffer = getBuffer(bufferFactory, gltfModel, primitive.attributes.at(WEIGHTS_STRING), flags);
            }
            if (hasJoints && hasWeights)
            {
                mesh->skinnedPositions = bufferFactory.createBuffer(mesh->vertexBuffers[VertexBufferType::POSITION]->info.size,
                                                                    flags,
                                                                    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
                mesh->skinnedNormals = bufferFactory.createBuffer(mesh->vertexBuffers[VertexBufferType::NORMAL]->info.size,
                                                                  flags,
                                                                  VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
            }

            // extract index data
            mesh->indexBuffer = getBuffer(bufferFactory, gltfModel, primitive.indices, vk::BufferUsageFlagBits::eIndexBuffer);

            meshMap[mesh.get()] = primitive.material;

            model->meshes.push_back(std::move(mesh));
            node->meshes.push_back(model->meshes.back().get());
        }
    }

    // recursively process child nodes
    for (int childIndex : gltfNode.children)
    {
        processNode(bufferFactory, gltfModel, model, node, childIndex, meshMap, numberNodeMap);
    }
}

void GLTFLoader::processSkinsAnimations(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model,
                                        tsl::robin_map<int, Node*> &numberNodeMap, int framesInFlight)
{
    // iterate through each skin
    for (auto &skin : gltfModel.skins)
    {
        model->skins.emplace_back(skin.name);
        Skin &skinObj = model->skins.back();

        const tinygltf::Accessor &accessor = gltfModel.accessors[skin.inverseBindMatrices];
        const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

        for (int i = 0; i < skin.joints.size(); i++)
        {
            int joint = skin.joints[i];
            if (accessor.type != TINYGLTF_TYPE_MAT4) throw std::runtime_error("Invalid inverse bind matrix type");
            // read inverse bind matrix
            const auto *ibm = reinterpret_cast<const glm::mat4*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset
                                                                              + i * sizeof(glm::mat4)]);

            auto n = numberNodeMap[joint];
            skinObj.nodeBindMatrices.push_back({ n, *ibm });
        }

        for (int i = 0; i < framesInFlight; i++)
            skinObj.jointBuffers.push_back(bufferFactory.createBuffer(skin.joints.size() * sizeof(glm::mat4),
                                                          vk::BufferUsageFlagBits::eStorageBuffer,
                                                          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT));
    }

    // get the raw animation data
    for (auto &animation : gltfModel.animations)
    {
        model->animations.emplace_back(animation.name);
        Animation &anim = model->animations.back();
        for (auto &channel : animation.channels)
        {
            TargetPath path;
            if (channel.target_path == "translation")
                path = TargetPath::TRANSLATION;
            else if (channel.target_path == "rotation")
                path = TargetPath::ROTATION;
            else if (channel.target_path == "scale")
                path = TargetPath::SCALE;
            else if (channel.target_path == "weights")
                path = TargetPath::WEIGHT;
            else
            {
                std::cerr << "Invalid target path " << channel.target_path << std::endl;
                throw std::runtime_error("Invalid target path");
            }

            anim.channels.push_back({ numberNodeMap[channel.target_node], channel.sampler, path });
        }
        for (auto &sampler : animation.samplers)
        {
            Interpolation interp = Interpolation::LINEAR;
            if (sampler.interpolation == "LINEAR")
                interp = Interpolation::LINEAR;
            else if (sampler.interpolation == "STEP")
                interp = Interpolation::STEP;
            else if (sampler.interpolation == "CUBICSPLINE")
                interp = Interpolation::CUBICSPLINE;
            else throw std::runtime_error("Invalid interpolation");

            anim.samplers.push_back({ sampler.input, sampler.output, interp });

            // copy raw animation data if not copied already
            auto addRawData = [&](int type) {

                const tinygltf::Accessor &accessor = gltfModel.accessors[type];
                const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

                if (model->animationRawData.contains(type)) return;

                const auto *data = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                model->animationRawData[type] =
                        std::vector<unsigned char>(data, data + accessor.count * accessor.ByteStride(bufferView));
            };

            addRawData(sampler.input);
            addRawData(sampler.output);
        }
    }
}

uPtr<Buffer> GLTFLoader::getBuffer(BufferFactory &bufferFactory, tinygltf::Model &gltfModel,
                           int type, vk::BufferUsageFlags flags)
{
    const tinygltf::Accessor &accessor = gltfModel.accessors[type];
    const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
    const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

    if (flags & vk::BufferUsageFlagBits::eIndexBuffer && flags & vk::BufferUsageFlagBits::eVertexBuffer)
    {
        std::cerr << "Cannot have both vertex and index buffer!" << std::endl;
        throw std::runtime_error("Invalid buffer usage flag");
    }

    uPtr<Buffer> vBuffer;

    if (flags & vk::BufferUsageFlagBits::eVertexBuffer)
    {
        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) throw std::runtime_error("int not supported");
        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE || accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
        {
            if (accessor.type != 4) throw std::runtime_error("Invalid type"); // assumes is always joint index
            vBuffer = bufferFactory.createBuffer(accessor.count * sizeof(glm::uvec4), flags,
                                                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
            {
                const auto *data = reinterpret_cast<const glm::u8vec4*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                std::vector<glm::uvec4> newData(accessor.count);
                for (int i = 0; i < accessor.count; i++)
                {
                    // hope this casts correctly
                    newData[i] = data[i];
                }
                vBuffer->fill(newData.data());
            }
            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
            {
                const auto *data = reinterpret_cast<const glm::u16vec4*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                std::vector<glm::uvec4> newData(accessor.count);
                for (int i = 0; i < accessor.count; i++)
                {
                    newData[i] = data[i];
                }
                vBuffer->fill(newData.data());
            }
        }
        else
        {
            // float or short
            vBuffer = bufferFactory.createBuffer(bufferView.byteLength, flags,
                                                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
            vBuffer->fill(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        }
    }

    if (flags & vk::BufferUsageFlagBits::eIndexBuffer)
    {

        // handle different formats
        if (accessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
        {
            // 16 bit
            vBuffer = bufferFactory.createBuffer(accessor.count * sizeof(uint16_t), flags,
                                                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
            vBuffer->setIndexType(vk::IndexType::eUint16);
        }
        else if (accessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT)
        {
            // 32 bit
            vBuffer = bufferFactory.createBuffer(accessor.count * sizeof(uint32_t), flags,
                                                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
            vBuffer->setIndexType(vk::IndexType::eUint32);
        }
        else
        {
            std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
            throw std::runtime_error("Index type not supported");
        }
        vBuffer->fill(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
    }

    return vBuffer;
}

void GLTFLoader::makeMaterialsAndTextures(CommandPool &commandPool, QueueManager &queueManager, BufferFactory &bufferFactory,
                                          tinygltf::Model &gltfModel, Model *model)
{
    auto time1 = std::chrono::high_resolution_clock::now();
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

        loadPercent.store(loadPercent.load() + 0.5f / gltfModel.images.size() * 0.3f);
    }

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
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16;
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
        loadPercent.store(loadPercent.load() + 0.5f / gltfModel.textures.size() * 0.1f);
    }

    for (const auto &mat : gltfModel.materials)
    {
        Material material;
        material.baseColorFactor = glm::vec4(mat.pbrMetallicRoughness.baseColorFactor[0],
                                             mat.pbrMetallicRoughness.baseColorFactor[1],
                                             mat.pbrMetallicRoughness.baseColorFactor[2],
                                             mat.pbrMetallicRoughness.baseColorFactor[3]);
        material.metallicFactor = (float)mat.pbrMetallicRoughness.metallicFactor;
        material.roughnessFactor = (float)mat.pbrMetallicRoughness.roughnessFactor;

        material.baseColorTexture = mat.pbrMetallicRoughness.baseColorTexture.index;
        material.metallicRoughnessTexture = mat.pbrMetallicRoughness.metallicRoughnessTexture.index;
        material.normalTexture = mat.normalTexture.index;

        material.occlusionTexture = mat.occlusionTexture.index;
        material.occlusionStrength = (float)mat.occlusionTexture.strength;
        material.occlusionUVSet = mat.occlusionTexture.texCoord;

        material.emissiveTexture = mat.emissiveTexture.index;
        material.emissiveFactor = glm::vec3(mat.emissiveFactor[0], mat.emissiveFactor[1], mat.emissiveFactor[2]);

        model->materials.push_back(material);
        loadPercent.store(loadPercent.load() + 0.5f / gltfModel.materials.size() * 0.1f);
    }

    // submit as an async batch to make it smoother
    // note: cannot submit to same queue on different threads
// TODO: if using two different queue families, it needs to be transferred. but validation is not complaining?
    commandPool.submitSingleTimeCommands(queueManager, commandBuffers, true);

    for (auto &buffer : stagingBuffers)
        buffer->destroy();

    auto time2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time1).count();
    std::cout << "Material and texture creation took " << duration << "ms" << std::endl;
}

uPtr<Buffer> GLTFLoader::createTangentVectors(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, int vertexType,
                                              int normalType, int uvType, int indexType, vk::BufferUsageFlags flags)
{
    auto getReinterpretedPointer = [](tinygltf::Model &gltfModel, int type) -> const void* {
        const tinygltf::Accessor &accessor = gltfModel.accessors[type];
        const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];
        return &buffer.data[bufferView.byteOffset + accessor.byteOffset];
    };

    const tinygltf::Accessor &vAccessor = gltfModel.accessors[vertexType];

    const tinygltf::Accessor &iAccessor = gltfModel.accessors[indexType];

    const auto* positions = reinterpret_cast<const glm::vec3*>(getReinterpretedPointer(gltfModel, vertexType));
    const auto* normals = reinterpret_cast<const glm::vec3*>(getReinterpretedPointer(gltfModel, normalType));
    const auto* uvs = reinterpret_cast<const glm::vec2*>(getReinterpretedPointer(gltfModel, uvType));
    const auto* indices16 = reinterpret_cast<const uint16_t*>(getReinterpretedPointer(gltfModel, indexType));
    const auto* indices32 = reinterpret_cast<const uint32_t*>(getReinterpretedPointer(gltfModel, indexType));

    auto buffer = bufferFactory.createBuffer(vAccessor.count * sizeof(glm::vec3), flags,
                                              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    auto *tangents = (glm::vec3*) malloc(vAccessor.count * sizeof(glm::vec3));

    VertexCountIndex vertexCountIndex{};
    vertexCountIndex.vertexCount = vAccessor.count;
    vertexCountIndex.indexCount = iAccessor.count;
    vertexCountIndex.isInd16 = iAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT;
    vertexCountIndex.vertices = const_cast<glm::vec3*>(positions);
    vertexCountIndex.normals = const_cast<glm::vec3*>(normals);
    vertexCountIndex.uvs = const_cast<glm::vec2*>(uvs);
    vertexCountIndex.ind16 = const_cast<uint16_t*>(indices16);
    vertexCountIndex.ind32 = const_cast<uint32_t*>(indices32);
    vertexCountIndex.tangents = tangents;

    TangentCalc tangentCalc(&gltfModel, &vertexCountIndex);
    tangentCalc.calculate();

    buffer->fill(vertexCountIndex.tangents);

    free(tangents);
    return buffer;
}

float GLTFLoader::getLoadPercent()
{
    return loadPercent.load();
}

void GLTFLoader::setLoadPercent(float percent)
{
    loadPercent.store(percent);
}
