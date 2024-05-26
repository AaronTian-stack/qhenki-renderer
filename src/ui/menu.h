#pragma once

class Menu
{
private:
    bool open = false;
public:
    virtual void renderMenu() = 0;
    friend class UserInterface;
};
