#pragma once

#include <atomic>
#include <gltf/tiny_gltf.h>
#include "model.h"
#include <smartpointer.h>
#include "../vulkan/buffer/bufferfactory.h"
#include <tsl/robin_map.h>

#define POSITION_STRING "POSITION"
#define NORMAL_STRING "NORMAL"
#define TEXCOORD_STRING_0 "TEXCOORD_0"
#define TEXCOORD_STRING_1 "TEXCOORD_1"
#define TANGENT_STRING "TANGENT"
#define JOINTS_STRING "JOINTS_0"
#define WEIGHTS_STRING "WEIGHTS_0"

class GLTFLoader
{
private:
    static inline std::atomic<float> loadPercent = 1.f; // TODO: rewrite so that loader is its own object (not static)

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
    static void processNode(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model, Node *parent, int nodeIndex,
                            tsl::robin_map<Mesh*, int> &meshMap, tsl::robin_map<int, Node*> &numberNodeMap);
    static void processSkinsAnimations(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model, tsl::robin_map<int, Node*> &numberNodeMap);
    static uPtr<Buffer> createTangentVectors(BufferFactory &bufferFactory, tinygltf::Model &gltfModel , int verticesType,
                                             int normalType, int uvType, int indexType, vk::BufferUsageFlags flags);
    static uPtr<Buffer> getBuffer(BufferFactory &bufferFactory, tinygltf::Model &gltfModel,
                          int type, vk::BufferUsageFlags flags);

public:
    static uPtr<Model> create(CommandPool &commandPool, QueueManager &queueManager, BufferFactory &bufferFactory, const char* filename);
    static float getLoadPercent();
    static void setLoadPercent(float percent);
};
