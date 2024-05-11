#pragma once

#include "gltf/tiny_gltf.h"
#include "model.h"
#include "../smartpointer.h"
#include "../vulkan/buffer/bufferfactory.h"

#define POSITION_STRING "POSITION"
#define NORMAL_STRING "NORMAL"
#define TEXCOORD_STRING "TEXCOORD_0"
#define TANGENT_STRING "TANGENT"

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

    static const inline std::array<std::pair<const char*, VertexBufferType>, 4> typeMap =
    {
        std::make_pair(POSITION_STRING, VertexBufferType::POSITION),
        std::make_pair(NORMAL_STRING, VertexBufferType::NORMAL),
        std::make_pair(TEXCOORD_STRING, VertexBufferType::UV),
        std::make_pair(TANGENT_STRING, VertexBufferType::TANGENT),
    };

    static void makeMaterialsAndTextures(CommandPool &commandPool, QueueManager &queueManager,
                                         BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model);
    static void processNode(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model, Node *parent, int nodeIndex);
    static uPtr<Buffer> createTangentVectors(BufferFactory &bufferFactory, tinygltf::Model &gltfModel , int verticesType,
                                             int normalType, int uvType, int indexType, vk::BufferUsageFlagBits flag);
    static uPtr<Buffer> getBuffer(BufferFactory &bufferFactory, tinygltf::Model &gltfModel,
                          int type, vk::BufferUsageFlagBits flag, size_t vertexSize);

public:
    static uPtr<Model> create(CommandPool &commandPool, QueueManager &queueManager, BufferFactory &bufferFactory, const char* filename);
    static LoadStatus getLoadStatus();
    static void setLoadStatus(LoadStatus status);
};
