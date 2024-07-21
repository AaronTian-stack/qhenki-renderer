#include "visualmenu.h"

VisualMenu::VisualMenu()
{}

void VisualMenu::renderMenu(void *payload)
{
    auto *visualMenuPayload = static_cast<VisualMenuPayload *>(payload);
    bool validPayload = visualMenuPayload != nullptr;
    if (open)
    {
        ImGui::Begin("Options", &open, ImGuiWindowFlags_AlwaysAutoResize);

        if (validPayload)
        {
            ImGui::Checkbox("Draw Background", visualMenuPayload->drawBackground);
            ImGui::DragFloat("Background Rotation", visualMenuPayload->cubeMapRotation, 1.f);
            ImGui::BeginDisabled(*visualMenuPayload->drawBackground);
            ImGui::ColorEdit3("Clear Color", reinterpret_cast<float *>(visualMenuPayload->clearColor));
            ImGui::EndDisabled();

            ImGui::Checkbox("Draw Grid", visualMenuPayload->drawGrid);
        }
        else
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Missing visual options");
        }

        ImGui::Separator();
        if (validPayload)
        {
            auto *lp = visualMenuPayload->lightingParameters;
            ImGui::DragFloat("IBL Intensity", &lp->iblIntensity, 0.01f, 0.f, 100.f);
            ImGui::DragFloat("Emission Multiplier", &lp->emissionMultiplier, 0.1f, 0.f, 100.f);
        }
        else
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Missing lighting parameters");
        }
        ImGui::Separator();
        if (ImGui::Button("Post-Processing Stack"))
        {
            if(validPayload) *visualMenuPayload->postProcessOpen = !*visualMenuPayload->postProcessOpen;
        }
        ImGui::SameLine();
        if (ImGui::Button("Lights"))
        {
            if(validPayload) *visualMenuPayload->lightsOpen = !*visualMenuPayload->lightsOpen;
        }
        ImGui::End();
    }
}
