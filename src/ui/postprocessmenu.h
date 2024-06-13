#pragma once

#include "menu.h"
#include "../vfx/postprocess.h"

class PostProcessMenu : public Menu
{
private:
    std::vector<std::pair<PostProcess*, bool>> openMap;
    void setOpen(PostProcess *postProcess, bool open);
    bool* isOpen(PostProcess *postProcess);
    void renderPostProcessMenu(PostProcess *postProcess);
public:
    void renderMenu(void *payload) override;
};
