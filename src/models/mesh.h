#pragma once

#include <vector>
#include "../vulkan/buffer/buffer.h"

enum VertexBufferType
{
    POSITION = 1 << 0,
    NORMAL = 1 << 1,
    COLOR = 1 << 2
};

class Mesh
{
private:
    std::vector<std::pair<uPtr<Buffer>, VertexBufferType>> vertexBuffers;
    uPtr<Buffer> indexBuffer;

public:
    Mesh();
    void draw(vk::CommandBuffer commandBuffer);
    void destroy();
    friend class GLTFLoader;
};
