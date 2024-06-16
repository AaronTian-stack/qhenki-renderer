#include "lightmenu.h"

void LightMenu::renderMenu(void *payload)
{
    if (open)
    {
        auto *lightsList = static_cast<LightsList*>(payload);
        ImGui::Begin("Lights", &open);
        if (lightsList)
        {
            // do a tree list for all the lights
            if (ImGui::CollapsingHeader("Sphere Lights"))
            {
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
                        if (ImGui::Button("Remove Light"))
                        {
                            lightsList->sphereLights->erase(lightsList->sphereLights->begin() + i);
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                // button to add another light
                if (ImGui::Button("Add Sphere Light"))
                {
                    lightsList->sphereLights->push_back({{}, {1.f, 1.f, 1.f}, 10.f, 1.f});
                }
            }

            // do a tree list for all the tube lights
            if (ImGui::CollapsingHeader("Tube Lights"))
            {
                for (int i = 0; i < lightsList->tubeLights->size(); i++)
                {
                    auto &light = (*lightsList->tubeLights)[i];
                    ImGui::PushID(i);
                    if (ImGui::TreeNode("Tube Light"))
                    {
                        ImGui::DragFloat3("Position", reinterpret_cast<float*>(&light.position), 0.01f);
                        // rotation
                        // TODO: drop down for rotation order
                        if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&light.eulerAngle), 0.1f, -180.f, 180.f))
                        {
                            // TODO: this sucks change this
                            auto er = glm::radians(light.eulerAngle);
                            // rotation matrix for each
                            auto x = glm::angleAxis(er.x, glm::vec3(1.f, 0.f, 0.f));
                            auto y = glm::angleAxis(er.y, glm::vec3(0.f, 1.f, 0.f));
                            auto z = glm::angleAxis(er.z, glm::vec3(0.f, 0.f, 1.f));
                            light.rotation = z * x * y;
                        }
                        ImGui::DragFloat("Length", &light.length, 0.01f, 0.f, 100.f);
                        ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&light.color));
                        ImGui::DragFloat("Intensity", &light.intensity, 0.5f, 0.f, 100.f);
                        ImGui::DragFloat("Radius", &light.radius, 0.01f, 0.f, 100.f);
                        if (ImGui::Button("Remove Light"))
                        {
                            lightsList->tubeLights->erase(lightsList->tubeLights->begin() + i);
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                if (ImGui::Button("Add Tube Light"))
                {
                    lightsList->tubeLights->push_back({{}, 0.2f, {1.f, 1.f, 1.f}, 10.f, 0.2f, {}, {}});
                }
            }
        }
        else
        {
            // red error
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Missing lights list");
        }
        ImGui::End();
    }
}
