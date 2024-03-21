// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include <iostream>
#include "gltfloader.h"
#include "glm/vec3.hpp"
#include "glm/gtc/type_ptr.hpp"

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
    // assume the root node is the scene
    int rootNodeIndex = gltfModel.defaultScene >= 0 ? gltfModel.defaultScene : 0;
    processNode(bufferFactory, gltfModel, model.get(), rootNodeIndex);

    return model;
}

void GLTFLoader::processNode(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model, int nodeIndex)
{
    const tinygltf::Node& node = gltfModel.nodes[nodeIndex];

    if (node.mesh >= 0)
    {
        const tinygltf::Mesh &gltfMesh = gltfModel.meshes[node.mesh];

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

                auto vBuffer = getBuffer(bufferFactory, gltfModel, primitive, primitive.attributes.at(type.first), vk::BufferUsageFlagBits::eVertexBuffer);
                mesh->vertexBuffers.emplace_back(std::move(vBuffer), type.second);
            }

            // extract index data
            mesh->indexBuffer = getBuffer(bufferFactory, gltfModel, primitive, primitive.indices, vk::BufferUsageFlagBits::eIndexBuffer);

            model->meshes.push_back(std::move(mesh));
        }
    }

    // recursively process child nodes
    for (int childIndex : node.children)
    {
        processNode(bufferFactory, gltfModel, model, childIndex);
    }
}

uPtr<Buffer> GLTFLoader::getBuffer(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, const tinygltf::Primitive &primitive,
                           int type, vk::BufferUsageFlagBits flag)
{
    const tinygltf::Accessor &accessor = gltfModel.accessors[type];
    const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
    const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

    size_t count = accessor.count;
    uPtr<Buffer> vBuffer;

    const float* positionBuffer = nullptr;
    std::vector<glm::vec3> data;

    switch (flag) {
        case vk::BufferUsageFlagBits::eVertexBuffer:
            positionBuffer = reinterpret_cast<const float*>(&buffer.data[0] + bufferView.byteOffset + accessor.byteOffset);

            for (size_t i = 0; i < count; ++i)
            {
                data.push_back(glm::make_vec3(positionBuffer + i * 3));
            }

            vBuffer = bufferFactory.createBuffer(count * sizeof(float), flag,
                                                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
            vBuffer->fill(data.data());
            break;
        case vk::BufferUsageFlagBits::eIndexBuffer:
            // technically indices can come in different formats, might need to handle that
            if (accessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
            {
                vBuffer = bufferFactory.createBuffer(count * sizeof(uint16_t), flag,
                                                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
                vBuffer->fill(&buffer.data[0] + bufferView.byteOffset + accessor.byteOffset);
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
