#include "modelmenu.h"
#include "../models/model.h"

void nodeInfo(Node *node)
{
    if (!node)
        return;
    auto flag = 0;
    auto &children = node->getChildren();
    if (children.empty())
    {
        flag = static_cast<ImGuiTreeNodeFlags_>(flag | ImGuiTreeNodeFlags_Leaf);
    }
    if (ImGui::TreeNodeEx(node->getName().c_str(), flag))
    {
        for (auto &child : children)
        {
            nodeInfo(child.get());
        }
        ImGui::TreePop();
    }
}

void ModelMenu::renderMenu(void *payload)
{
    if (open)
    {
        ImGui::Begin("Model Inspector", &open);

        auto model = static_cast<Model*>(payload);
        if (!model)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No model selected");
        }
        else {
            // display animations
            auto &animations = model->getAnimations();
            auto ind = model->getAnimationIndex();
            // drop down menu to select animation
            if (!animations.empty())
            {
                if (ImGui::BeginCombo("Selected Animation", animations[ind].name.c_str())) {
                    for (int i = 0; i < animations.size(); i++) {
                        // highlight selected animation
                        if (i == ind) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                        }
                        else {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        }

                        if (ImGui::Selectable(animations[i].name.c_str())) {
                            // set animation
                            model->setAnimationIndex(i);
                        }

                        ImGui::PopStyleColor();
                    }
                    ImGui::EndCombo();
                }
            }
            else
            {
                ImGui::BeginDisabled();
                if (ImGui::BeginCombo("Selected Animation", "NO ANIMATIONS"))
                {
                    ImGui::EndCombo();
                }
                ImGui::EndDisabled();
            }
            ImGui::Separator();
            // display node graph using imgui treenode
            ImGui::Text("Node Hierarchy");
            nodeInfo(model->getRoot());
        }

        ImGui::End();
    }
}
