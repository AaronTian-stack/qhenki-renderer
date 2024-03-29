#pragma once

#include <vector>
#include "../vulkan/buffer/buffer.h"

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
    int materialIndex;

public:
    Mesh();
    void draw(vk::CommandBuffer commandBuffer);
    void destroy();
    friend class GLTFLoader;
};
