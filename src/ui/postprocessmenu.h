#pragma once

#include "menu.h"
#include "../vfx/postprocess.h"
#include "tsl/robin_set.h"

class PostProcessMenu : public Menu
{
private:
    tsl::robin_set<PostProcess*> processRenderedMenus;
    std::vector<std::pair<PostProcess*, bool>> openMap;
    void setOpen(PostProcess *postProcess, bool open);
    bool* isOpen(PostProcess *postProcess);
    void renderPostProcessMenu(PostProcess *postProcess);
public:
    void renderMenu(void *payload) override;
};
