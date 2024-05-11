#pragma once
#include "glm/glm.hpp"

struct Material
{
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    int baseColorTexture = -1;

    float metallicFactor = -1;
    float roughnessFactor = -1;
    int metallicRoughnessTexture = -1;

    int normalTexture = -1;

    int occlusionTexture = -1;
    float occlusionStrength = 1.0f;

    int emissiveTexture = -1;
    glm::vec3 emissiveFactor = glm::vec3(0.0f);
};
