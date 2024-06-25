#pragma once

#include "menu.h"
#include "../lights/lights.h"
#include <glm/glm.hpp>

struct VisualMenuPayload
{
    LightingParameters *lightingParameters;
    glm::vec3 *clearColor;
    bool *postProcessOpen;
    bool *lightsOpen;
    bool *drawBackground;
};

class VisualMenu : public Menu
{
public:
    VisualMenu();
    void renderMenu(void *payload) override;
};
