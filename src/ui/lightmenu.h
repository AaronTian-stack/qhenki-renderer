#pragma once

#include "menu.h"
#include "../lights/lights.h"
#include <vector>

struct LightsList
{
    std::vector<SphereLight> *sphereLights;
    std::vector<TubeLight> *tubeLights;
    std::vector<RectangleLight> *rectangleLights;
};

class LightMenu : public Menu
{
public:
    void renderMenu(void *payload) override;
};

void handleRotations(glm::quat &q, glm::vec3 &euler);
