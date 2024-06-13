#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

struct LightingParameters
{
    float iblIntensity = 1.f;
    float emissionMultiplier = 1.f;
    int pointLightCount;
    int sphereLightCount;
    int tubeLightCount;
    int rectangleLightCount;
};

struct PointLight
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

struct SphereLight
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float radius;
};

struct TubeLight
{
    glm::vec3 position; // end points are in x direction from this
    float length;
    glm::vec3 color;
    float intensity;
    float radius;
    glm::quat rotation;
};

struct TubeLightShader
{
    glm::vec3 position1;
    glm::vec3 position2;
    glm::vec3 color;
    float radius;
};