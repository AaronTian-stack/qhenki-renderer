#include <vector>
#include "../smartpointer.h"
#include "mesh.h"
#include "glm/detail/type_mat4x4.hpp"
#include "node.h"
#include "../vulkan/texture/image.h"
#include "../vulkan/texture/texture.h"

class Model
{
private:
    std::vector<uPtr<Mesh>> meshes;

    std::vector<uPtr<Texture>> textures;
    std::vector<uPtr<Image>> images;

public:
    uPtr<Node> root;
    Model();
    void draw(vk::CommandBuffer commandBuffer);
    void destroy();
    friend class GLTFLoader;
};
