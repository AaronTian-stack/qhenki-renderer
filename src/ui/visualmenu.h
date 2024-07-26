#pragma once

#include "menu.h"
#include "../lights/lights.h"
#include <glm/glm.hpp>

struct VisualMenuPayload
{
    LightingParameters *lightingParameters;
    float *cubeMapRotation;
    glm::vec3 *clearColor;
    bool *postProcessOpen;
    bool *lightsOpen;
    bool *drawBackground;
    bool *drawGrid;
    float *gridScale;
};

class VisualMenu : public Menu
{
public:
    VisualMenu();
    void renderMenu(void *payload) override;
};
