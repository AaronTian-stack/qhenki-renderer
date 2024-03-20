#include <vector>
#include "../smartpointer.h"
#include "mesh.h"

class Model
{
private:
    std::vector<uPtr<Mesh>> meshes;
public:
    Model();
    void draw(vk::CommandBuffer commandBuffer);
    void destroy();
    friend class GLTFLoader;
};
