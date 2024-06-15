#include "lightmenu.h"
#include <glm/gtc/matrix_transform.hpp>

void LightMenu::renderMenu(void *payload)
{
    if (open)
    {
        auto *lightsList = static_cast<LightsList*>(payload);

        ImGui::Begin("Lights", &open);

        if (lightsList)
        {
            // do a tree list for all the lights
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            if (ImGui::TreeNode("Sphere Lights"))
            {
                ImGui::PopStyleColor();
                for (int i = 0; i < lightsList->sphereLights->size(); i++)
                {
                    auto &light = (*lightsList->sphereLights)[i];
                    ImGui::PushID(i);
                    if (ImGui::TreeNode("Sphere Light"))
                    {
                        ImGui::DragFloat3("Position", reinterpret_cast<float*>(&light.position), 0.01f);
                        ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&light.color));
                        ImGui::DragFloat("Intensity", &light.intensity, 0.5f, 0.f, 100.f);
                        ImGui::DragFloat("Radius", &light.radius, 0.01f, 0.f, 100.f);
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            else ImGui::PopStyleColor();

            // do a tree list for all the tube lights
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            if (ImGui::TreeNode("Tube Lights"))
            {
                ImGui::PopStyleColor();
                for (int i = 0; i < lightsList->tubeLights->size(); i++)
                {
                    auto &light = (*lightsList->tubeLights)[i];
                    ImGui::PushID(i);
                    if (ImGui::TreeNode("Tube Light"))
                    {
                        ImGui::DragFloat3("Position", reinterpret_cast<float*>(&light.position), 0.01f);
                        // rotation
                        // TODO: drop down for rotation order
                        if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&light.eulerAngle), 0.1f))
                        {
                            // TODO: this sucks change this
                            auto er = glm::radians(light.eulerAngle);
                            // rotation matrix for each
                            auto x = glm::rotate(glm::mat4(1.0f), er.x, glm::vec3(1.f, 0.f, 0.f));
                            auto y = glm::rotate(glm::mat4(1.0f), er.y, glm::vec3(0.f, 1.f, 0.f));
                            auto z = glm::rotate(glm::mat4(1.0f), er.z, glm::vec3(0.f, 0.f, 1.f));
                            auto o = z * y * x;
                            light.rotation = glm::quat_cast(o);
                        }
                        ImGui::DragFloat("Length", &light.length, 0.01f, 0.f, 100.f);
                        ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&light.color));
                        ImGui::DragFloat("Intensity", &light.intensity, 0.5f, 0.f, 100.f);
                        ImGui::DragFloat("Radius", &light.radius, 0.01f, 0.f, 100.f);
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            else ImGui::PopStyleColor();
        }
        else
        {
            // red error
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Missing lights list");
        }
        ImGui::End();
    }
}
