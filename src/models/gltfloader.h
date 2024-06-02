#pragma once

#include <atomic>
#include "gltf/tiny_gltf.h"
#include "model.h"
#include "../smartpointer.h"
#include "../vulkan/buffer/bufferfactory.h"

#define POSITION_STRING "POSITION"
#define NORMAL_STRING "NORMAL"
#define TEXCOORD_STRING_0 "TEXCOORD_0"
#define TEXCOORD_STRING_1 "TEXCOORD_1"
#define TANGENT_STRING "TANGENT"

class GLTFLoader
{
private:
    static inline std::atomic<float> loadPercent = 1.f;

    static const inline std::array<std::pair<const char*, VertexBufferType>, 5> typeMap =
    {
        std::make_pair(POSITION_STRING, VertexBufferType::POSITION),
        std::make_pair(NORMAL_STRING, VertexBufferType::NORMAL),
        std::make_pair(TEXCOORD_STRING_0, VertexBufferType::UV_0),
        std::make_pair(TEXCOORD_STRING_1, VertexBufferType::UV_1),
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
    static float getLoadPercent();
    static void setLoadPercent(float percent);
};
