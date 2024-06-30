#pragma once

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
    uPtr<Node> root;
    std::vector<uPtr<Mesh>> meshes;

    std::vector<Material> materials;
    std::vector<uPtr<Texture>> textures; // >= num images
    std::vector<uPtr<Image>> images;

    std::vector<Skin> skins;
    std::vector<Animation> animations;
    tsl::robin_map<int, std::vector<unsigned char>> animationRawData;

public:
    Model();
    Node* getRoot();
    std::vector<vk::DescriptorImageInfo> getDescriptorImageInfo();
    void skin(vk::CommandBuffer commandBuffer);
    void destroy();
    void updateAnimation(float time);

    friend class Node;
    friend class GLTFLoader;
};
