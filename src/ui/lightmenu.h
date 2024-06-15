#pragma once

#include "menu.h"
#include "../lights/lights.h"
#include <vector>

struct LightsList
{
    std::vector<SphereLight> *sphereLights;
    std::vector<TubeLight> *tubeLights;
};

class LightMenu : public Menu
{
public:
    void renderMenu(void *payload) override;
};
