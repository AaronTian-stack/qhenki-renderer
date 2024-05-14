#pragma once

#include <vector>
#include "../vulkan/buffer/buffer.h"
#include "material.h"

class Model;

enum VertexBufferType
{
    POSITION,
    NORMAL,
    TANGENT,
    UV_0,
    UV_1,
};

class Mesh
{
private:;
    int materialIndex; // used in loading only TODO: get rid of this
    std::array<uPtr<Buffer>, 5> vertexBuffers;

    uPtr<Buffer> indexBuffer;

public:
    Material *material;
    Mesh();
    void draw(vk::CommandBuffer commandBuffer);
    void destroy();
    friend class GLTFLoader;
};
