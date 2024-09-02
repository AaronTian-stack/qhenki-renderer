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
    int animationIndex = 0;

public:
    Model();
    Node* getRoot();
    std::vector<vk::DescriptorImageInfo> getDescriptorImageInfo();
    void destroy();
    void updateAnimation(float time, int frame);
    const std::vector<Animation>& getAnimations() const { return animations; }

    int getAnimationIndex() const { return animationIndex; }
    void setAnimationIndex(int index) { animationIndex = index; }

    friend class Node;
    friend class GLTFLoader;
};
