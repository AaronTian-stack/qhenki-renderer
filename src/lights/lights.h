#pragma once

#include "glm/detail/type_mat4x4.hpp"

struct LightingParameters
{
    float iblIntensity;
    float emissionMultiplier;
    int pointLightCount;
    int sphereLightCount;
    int tubeLightCount;
    int rectangleLightCount;
};

struct PointLight
{
    glm::vec3 position;
    glm::vec3 color;
};

struct SphereLight
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float radius;
};
