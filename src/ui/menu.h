#pragma once

#include "imgui/imgui.h"

class Menu
{
protected:
    bool open;

public:
    Menu() : open(false) {}
    virtual void renderMenu(void *payload) = 0;
    friend class UserInterface;
};
