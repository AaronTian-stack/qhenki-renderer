#include <vector>
#include "../vulkan/buffer.h"

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
    //unsigned int indexCount;

public:
    Mesh();
    void draw(vk::CommandBuffer commandBuffer);
    void destroy();
    friend class GLTFLoader;
};
