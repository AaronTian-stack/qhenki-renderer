#include "../vulkan/texture/texture.h"
#include "glm/vec4.hpp"

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

    int emissiveTexture;
};
