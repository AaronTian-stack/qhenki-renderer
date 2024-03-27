#pragma once

#include "gltf/tiny_gltf.h"
#include "model.h"
#include "../smartpointer.h"
#include "../vulkan/buffer/bufferfactory.h"

class GLTFLoader
{
private:
    static const inline std::unordered_map<const char*, VertexBufferType> typeMap =
    {
        {"POSITION", VertexBufferType::POSITION},
        {"NORMAL", VertexBufferType::NORMAL},
        {"COLOR", VertexBufferType::COLOR},
        {"TEXCOORD_0", VertexBufferType::UV}
    };
    static void makeTextures(CommandPool &commandPool, QueueManager &queueManager,BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model);
    static void processNode(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model, Node *parent, int nodeIndex);
    static uPtr<Buffer> getBuffer(BufferFactory &bufferFactory, tinygltf::Model &gltfModel,
                          int type, vk::BufferUsageFlagBits flag, size_t vertexSize);

public:
    static uPtr<Model> load(BufferFactory &bufferFactory, const char* filename);
};
