#pragma once
#include "glm/glm.hpp"

struct Material
{
    glm::vec4 baseColorFactor;
    int baseColorTexture;

    float metallicFactor;
    float roughnessFactor;
    int metallicRoughnessTexture;

    int normalTexture;

    int occlusionTexture;
    float occlusionStrength;
    int occlusionUVSet;

    int emissiveTexture;
    glm::vec3 emissiveFactor;
};
