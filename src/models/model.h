#include <vector>
#include "../smartpointer.h"
#include "mesh.h"
#include "glm/detail/type_mat4x4.hpp"
#include "node.h"
#include "../vulkan/texture/image.h"
#include "../vulkan/texture/texture.h"
#include "material.h"

class Model
{
private:
    std::vector<uPtr<Mesh>> meshes;

    std::vector<Material> materials;
    std::vector<uPtr<Texture>> textures; // >= num images
    std::vector<uPtr<Image>> images;

public:
    uPtr<Node> root;
    Model();
    std::vector<vk::DescriptorImageInfo> getDescriptorImageInfo();
    void draw(vk::CommandBuffer commandBuffer);
    void destroy();
    friend class GLTFLoader;
};
