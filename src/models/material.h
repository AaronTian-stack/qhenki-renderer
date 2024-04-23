#pragma once

#include "glm/vec4.hpp"

struct Material
{
    alignas(16) glm::vec4 baseColorFactor;
    alignas(4) int baseColorTexture;

    alignas(4) float metallicFactor;
    alignas(4) float roughnessFactor;
    alignas(4) int metallicRoughnessTexture;

    alignas(4) int normalTexture;

    alignas(4) int occlusionTexture;
    alignas(4) float occlusionStrength;

    alignas(4) int emissiveTexture;
    alignas(16) glm::vec4 emissiveFactor;
};
