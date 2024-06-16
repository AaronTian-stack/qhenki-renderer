#include "postprocessmenu.h"
#include "../vfx/postprocessmanager.h"
#include "../vfx/effects/fxaa.h"
#include "../vfx/effects/sharpen.h"
#include "../vfx/effects/vignette.h"
#include "../vfx/effects/filmgrain.h"

void PostProcessMenu::renderMenu(void *payload)
{
    if (open)
    {
        ImGui::Begin("Post-Processing Stack", &open, ImGuiWindowFlags_AlwaysAutoResize);

        auto postProcessManager = static_cast<PostProcessManager *>(payload);
        if (!postProcessManager)
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "PostProcessManager missing");
            ImGui::End();
            return;
        }

        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(200.0f);
            ImGui::Text(
                    "Drag and drop effects to apply. Note that order matters, and that duplicates of the same effect will share the same parameters.");
            ImGui::EndTooltip();
        }

        if (ImGui::BeginTable("table1", 2))
        {
            float columnWidth = 120.f;
            ImGui::TableSetupColumn("Available Effects", ImGuiTableColumnFlags_WidthFixed, columnWidth);
            ImGui::TableSetupColumn("Active Effects", ImGuiTableColumnFlags_WidthFixed, columnWidth);

            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            // list box
            auto &postProcesses = postProcessManager->getPostProcesses();

            static int item_current_idx = 0;
            float listboxHeight = ImGui::GetTextLineHeightWithSpacing() * 10;
            if (ImGui::BeginListBox("##listbox 1", ImVec2(-FLT_MIN, listboxHeight)))
            {
                for (int i = 0; i < postProcesses.size(); i++)
                {
                    ImGui::PushID(i);
                    ImGui::Selectable(postProcesses[i]->name);

                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        ImGui::SetDragDropPayload("DND", &i, sizeof(int)); // post process index

                        ImGui::Text("%s", postProcesses[i]->name);
                        ImGui::EndDragDropSource();
                    }

                    ImGui::PopID();
                }
                ImGui::EndListBox();
            }
            ImGui::TableNextColumn();

            auto &toneMappers = postProcessManager->getToneMappers();
            auto activePostProcesses = postProcessManager->getActivePostProcesses();
            if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, listboxHeight)))
            {
                ImGui::SetNextItemWidth(columnWidth * 0.95f);
                const auto activeTM = postProcessManager->getActiveToneMapper();

                if (ImGui::BeginCombo("##combo", activeTM->name))
                {
                    for (int i = 0; i < toneMappers.size(); i++)
                    {
                        bool is_selected = (activeTM == toneMappers[i].get());
                        if (ImGui::Selectable(toneMappers[i]->name, is_selected))
                        {
                            postProcessManager->setToneMapper(i);
                        }
                    }
                    ImGui::EndCombo();
                }
                if (ImGui::BeginItemTooltip())
                {
                    ImGui::Text("Active Tonemapper");
                    ImGui::EndTooltip();
                }

                int indexToRemove = -1;
                for (int i = 0; i < activePostProcesses.size(); i++)
                {
                    ImGui::PushID(i);
                    if (ImGui::Selectable(activePostProcesses[i]->name))
                    {
                        setOpen(activePostProcesses[i], true);
                    }
                    ImGui::PopID();
                }
                for (int i = 0; i < activePostProcesses.size(); i++)
                {
                    ImGui::PushID(i);
                    bool *open = isOpen(activePostProcesses[i]);
                    if (open && *open)
                    {
                        ImGui::Begin(activePostProcesses[i]->name, open, ImGuiWindowFlags_AlwaysAutoResize);
                        renderPostProcessMenu(activePostProcesses[i]);
                        if (ImGui::Button("Remove"))
                        {
                            indexToRemove = i;
                            *open = false;
                        }
                        ImGui::End();
                    }
                    ImGui::PopID();
                }
                if (indexToRemove != -1)
                {
                    postProcessManager->deactivatePostProcess(indexToRemove);
                }
                ImGui::EndListBox();
            }
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *pd = ImGui::AcceptDragDropPayload("DND"))
                {
                    IM_ASSERT(pd->DataSize == sizeof(int));
                    int index = *(const int *) pd->Data;
                    postProcessManager->activatePostProcess(index);
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }
}

void PostProcessMenu::renderPostProcessMenu(PostProcess *postProcess)
{
    auto *fxaa = dynamic_cast<FXAA*>(postProcess);
    if (fxaa)
    {
        ImGui::SliderFloat("Reduce Mul", &fxaa->fxaaData.fxaaReduceMul, 0.0f, 256.0f);
        ImGui::SliderFloat("Reduce Min", &fxaa->fxaaData.fxaaReduceMin, 0.0f, 16.0f);
        ImGui::SliderFloat("Span Max", &fxaa->fxaaData.fxaaSpanMax, 0.0f, 16.0f);
    }
    auto *sharpen = dynamic_cast<Sharpen*>(postProcess);
    if (sharpen)
    {
        ImGui::SliderFloat("Intensity", &sharpen->intensity, 0.0f, 2.0f);
    }
    auto *vignette = dynamic_cast<Vignette*>(postProcess);
    if (vignette)
    {
        ImGui::SliderFloat("Intensity", &vignette->vignetteData.intensity, 0.0f, 2.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::SliderFloat("Outer Ring", &vignette->vignetteData.vignetteX, 0.0f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
        {
            if (vignette->vignetteData.vignetteX <= vignette->vignetteData.vignetteY)
                vignette->vignetteData.vignetteY = vignette->vignetteData.vignetteX - 0.001f;
        }
        if (ImGui::SliderFloat("Inner Falloff", &vignette->vignetteData.vignetteY, 0.0f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
        {
            if (vignette->vignetteData.vignetteY >= vignette->vignetteData.vignetteX)
                vignette->vignetteData.vignetteX = vignette->vignetteData.vignetteY + 0.001f;
        }
        ImGui::DragFloat("Saturation", &vignette->vignetteData.saturation, 0.01f, 10.f);
        ImGui::DragFloat("Saturation Mul", &vignette->vignetteData.saturationMul, 0.01f, 10.f);
    }
    auto *filmGrain = dynamic_cast<FilmGrain*>(postProcess);
    if (filmGrain)
    {
        ImGui::SliderFloat("Intensity", &filmGrain->filmGrainData.intensity, 0.0f, 1.0f);
    }
}

void PostProcessMenu::setOpen(PostProcess *postProcess, bool open)
{
    for (auto &pair : openMap)
    {
        if (pair.first == postProcess)
        {
            pair.second = open;
            return;
        }
    }
    openMap.emplace_back(postProcess, open);
}

bool* PostProcessMenu::isOpen(PostProcess *postProcess)
{
    for (auto &pair : openMap)
    {
        if (pair.first == postProcess)
        {
            return &pair.second;
        }
    }
    return nullptr;
}
