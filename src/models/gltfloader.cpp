// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include <iostream>
#include "gltfloader.h"
#include "glm/vec3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"

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
    //int rootNodeIndex = gltfModel.defaultScene >= 0 ? gltfModel.defaultScene : 0;
    int rootNodeIndex = gltfModel.scenes[gltfModel.defaultScene].nodes[0];

//    glm::quat quaternion(gltfModel.nodes[rootNodeIndex].rotation[0], gltfModel.nodes[rootNodeIndex].rotation[1],
//                         gltfModel.nodes[rootNodeIndex].rotation[2], gltfModel.nodes[rootNodeIndex].rotation[3]);
//    model->transform = glm::mat4_cast(quaternion);

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

                auto vBuffer = getBuffer(bufferFactory, gltfModel, primitive, primitive.attributes.at(type.first), vk::BufferUsageFlagBits::eVertexBuffer);
                mesh->vertexBuffers.emplace_back(std::move(vBuffer), type.second);
            }

            // extract index data
            mesh->indexBuffer = getBuffer(bufferFactory, gltfModel, primitive, primitive.indices, vk::BufferUsageFlagBits::eIndexBuffer);

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

uPtr<Buffer> GLTFLoader::getBuffer(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, const tinygltf::Primitive &primitive,
                           int type, vk::BufferUsageFlagBits flag)
{
    const tinygltf::Accessor &accessor = gltfModel.accessors[type];
    const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
    const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

    size_t count = accessor.count;
    uPtr<Buffer> vBuffer;

    switch (flag) {
        case vk::BufferUsageFlagBits::eVertexBuffer:
            vBuffer = bufferFactory.createBuffer(count * sizeof(glm::vec3), flag,
                                                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
            vBuffer->fill(&buffer.data[0] + bufferView.byteOffset + accessor.byteOffset);
            break;
        case vk::BufferUsageFlagBits::eIndexBuffer:
            // technically indices can come in different formats, might need to handle that
            if (accessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
            {
                // 16 bit
                vBuffer = bufferFactory.createBuffer(count * sizeof(uint16_t), flag,
                                                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
                vBuffer->fill(&buffer.data[0] + bufferView.byteOffset + accessor.byteOffset);
                vBuffer->indexType = vk::IndexType::eUint16;
            }
            else if (accessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT)
            {
                // 32 bit
                vBuffer = bufferFactory.createBuffer(count * sizeof(uint32_t), flag,
                                                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
                vBuffer->fill(&buffer.data[0] + bufferView.byteOffset + accessor.byteOffset);
                vBuffer->indexType = vk::IndexType::eUint32;
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
