#include <vector>
#include <smartpointer.h>
#include "mesh.h"
#include <glm/detail/type_mat4x4.hpp>
#include "node.h"
#include "../vulkan/texture/image.h"
#include "../vulkan/texture/texture.h"
#include "material.h"
#include "skin.h"
#include "animation.h"

class Model
{
private:
    std::vector<uPtr<Mesh>> meshes;

    std::vector<Material> materials;
    std::vector<uPtr<Texture>> textures; // >= num images
    std::vector<uPtr<Image>> images;

    std::vector<Skin> skins;
    std::vector<Animation> animations;
    tsl::robin_map<int, std::vector<unsigned char>> animationRawData;

    // each channel means the target node is animated
    // the channel refers to the sampler, which refers to input output buffers

    // ANIMATION BUFFERS (robin map to them)
    // translation 3 vec
    // rotation 4 quat
    // scale 3 vec

    // step 1: parse all animation sampler buffers in robin map
    // step 2: map input output from numbers to pointers

    // a node may or may not have a JOINT (which consists of inverse bind matrix, transform)

    // loop through all channels
        // get the data from sampler
    // update the node animated transform (this is joint local transform)
    // recalculate the world transform of the joint
    // send the data to the shader!


public:
    uPtr<Node> root;
    Model();
    std::vector<vk::DescriptorImageInfo> getDescriptorImageInfo();
    void draw(vk::CommandBuffer commandBuffer);
    void destroy();
    void updateAnimation(float time);
    friend class GLTFLoader;
};
