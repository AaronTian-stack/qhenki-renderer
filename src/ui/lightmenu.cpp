#include "lightmenu.h"

void LightMenu::renderMenu(void *payload)
{
    if (open)
    {
        ImGui::Begin("Lights", &open, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Light Menu");

        ImGui::End();
    }
}
