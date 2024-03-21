#include <vector>
#include "../smartpointer.h"
#include "mesh.h"
#include "glm/detail/type_mat4x4.hpp"
#include "node.h"

class Model
{
private:
    std::vector<uPtr<Mesh>> meshes;

public:
    uPtr<Node> root;
    Model();
    void draw(vk::CommandBuffer commandBuffer);
    void destroy();
    friend class GLTFLoader;
};
