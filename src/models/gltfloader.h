#pragma once

#include "gltf/tiny_gltf.h"
#include "model.h"
#include "../smartpointer.h"
#include "../vulkan/bufferfactory.h"

class GLTFLoader
{
private:
    static const inline std::unordered_map<const char*, VertexBufferType> typeMap =
    {
        {"POSITION", VertexBufferType::POSITION},
        {"NORMAL", VertexBufferType::NORMAL},
        {"COLOR", VertexBufferType::COLOR}
    };
    static void processNode(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, Model *model, Node *parent, int nodeIndex);
    static uPtr<Buffer> getBuffer(BufferFactory &bufferFactory, tinygltf::Model &gltfModel, const tinygltf::Primitive &primitive,
                          int type, vk::BufferUsageFlagBits flag);

public:
    static uPtr<Model> load(BufferFactory &bufferFactory, const char* filename);
};
