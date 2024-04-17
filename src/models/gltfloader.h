#pragma once

#include "gltf/tiny_gltf.h"
#include "model.h"
#include "../smartpointer.h"
#include "../vulkan/buffer/bufferfactory.h"

enum LoadStatus
{
    READY,
    PARSING,
    LOADING,
    LOADED_MAT_TEX,
    LOADED_NODE
};

class GLTFLoader
{
private:
    static inline std::atomic<LoadStatus> loadStatus = LoadStatus::READY;

    static const inline std::vector<std::pair<const char*, VertexBufferType>> typeMap =
    {
        {"POSITION", VertexBufferType::POSITION},
        {"NORMAL", VertexBufferType::NORMAL},
        {"TEXCOORD_0", VertexBufferType::UV},
        {"TANGENT", VertexBufferType::TANGENT},
    };
    static void makeMaterialsAndTextures(CommandPool &commandPool, QueueManager &queueManager, BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model);
    static void processNode(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model, Node *parent, int nodeIndex);
    static uPtr<Buffer> createTangentVectors(BufferFactory &bufferFactory, tinygltf::Model &gltfModel , int verticesType,
                                             int uvType, vk::BufferUsageFlagBits flag);
    static uPtr<Buffer> getBuffer(BufferFactory &bufferFactory, tinygltf::Model &gltfModel,
                          int type, vk::BufferUsageFlagBits flag, size_t vertexSize);

public:
    static uPtr<Model> create(CommandPool &commandPool, QueueManager &queueManager, BufferFactory &bufferFactory, const char* filename);
    static LoadStatus getLoadStatus();
    static void setLoadStatus(LoadStatus status);
};
