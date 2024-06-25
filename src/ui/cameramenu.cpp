#include "cameramenu.h"
#include "../inputprocesser.h"
#include "../camera.h"

void CameraMenu::renderMenu(void *payload)
{
    if (open)
    {
        ImGui::Begin("Camera Options", &open, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Sensitivity");
        ImGui::Separator();
        ImGui::SliderFloat("Rotate Sensitivity", &InputProcesser::SENSITIVITY_ROTATE, 0.0f, 0.2f);
        ImGui::SliderFloat("Translate Sensitivity", &InputProcesser::SENSITIVITY_TRANSLATE, 0.0f, 0.2f);
        ImGui::SliderFloat("Zoom Sensitivity", &InputProcesser::SENSITIVITY_ZOOM, 0.0f, 0.2f);
        ImGui::SliderFloat("FOV Sensitivity", &InputProcesser::SENSITIVITY_FOV, 0.0f, 20.0f);

        ImGui::Separator();
        // try to cast payload to camera
        auto *camera = static_cast<Camera *>(payload);
        if (camera == nullptr)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Camera missing");
        }
        else
        {
            ImGui::SliderFloat("FOV", &camera->regular.fov, 1.0f, 120.0f);
        }

        ImGui::End();
    }
}
