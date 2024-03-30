#pragma once

#include <vector>
#include "../vulkan/buffer/buffer.h"
#include "material.h"

enum VertexBufferType
{
    POSITION = 1 << 0,
    NORMAL = 1 << 1,
    UV = 1 << 2,
    COLOR = 1 << 3,
};

class Mesh
{
private:
    std::vector<std::pair<uPtr<Buffer>, VertexBufferType>> vertexBuffers;
    uPtr<Buffer> indexBuffer;

public:
    Material *material;
    Mesh();
    void draw(vk::CommandBuffer commandBuffer);
    void destroy();
    friend class GLTFLoader;
};
