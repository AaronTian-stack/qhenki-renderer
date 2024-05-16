#include "userinterface.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include <vulkan/vulkan.h>
#include <iterator>
#include <filesystem>
#include "inputprocesser.h"
#include <iostream>
#include "ImGuiFileDialog-0.6.7/ImGuiFileDialog.h"
#include "models/gltfloader.h"
#include <cstdlib>

UserInterface::UserInterface() {}

UserInterface::~UserInterface()
{
    vkDeviceWaitIdle(device); // wait for operations to finish before disposing
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(device, imguiPool, nullptr); // must happen after ImGui_ImplVulkan_Shutdown
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UserInterface::create(ImGuiCreateParameters param, CommandPool &commandPool)
{
    this->device = param.context->device.logicalDevice;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    const char* path = "../resources/fonts/Roboto-Medium.ttf";
    if (std::filesystem::exists(path))
        io.Fonts->AddFontFromFileTTF(path, 16);
    else
        std::cerr << "Could not find font" << std::endl;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    VkDescriptorPoolSize pool_sizes[] =
    {
    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create imgui descriptor pool!");
    }

    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForVulkan(param.window->getWindow(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = param.context->vkbInstance.instance;
    init_info.PhysicalDevice = param.context->device.physicalDevice;
    init_info.Device = param.context->device.logicalDevice;
    init_info.Queue = param.context->queueManager.queuesIndices.graphics;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = param.framesInFlight;
    init_info.ImageCount = param.framesInFlight;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    window = param.window;

    ImGui_ImplVulkan_Init(&init_info, param.renderPass->getRenderPass());

    auto commandBuffer = commandPool.beginSingleCommand();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    commandBuffer.end();
    commandPool.submitSingleTimeCommands(param.context->queueManager, {commandBuffer}, true);
}

void UserInterface::destroy()
{
    vkDeviceWaitIdle(device); // wait for operations to finish before disposing
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(device, imguiPool, nullptr); // must happen after ImGui_ImplVulkan_Shutdown
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UserInterface::render()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 5.0f;

    const int y = 21;
//    ImGui::ShowDemoWindow();
    ImGui::SetNextWindowPos(ImVec2(0, y));
    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

    ImGui::Begin("Stats", nullptr, flags);

    ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);

    ImGui::Text("Frame Time: %f ms", ImGui::GetIO().DeltaTime * 1000.f);

    frameTimes.push_back(ImGui::GetIO().DeltaTime);
    if (frameTimes.size() > 100)
        frameTimes.erase(frameTimes.begin());

    ImGui::PlotLines("", frameTimes.data(), frameTimes.size(), 0, "",
                     0.f, 0.05f, ImVec2(0, 20));

    ImGui::Separator();
    if (ImGui::Button("Visual Options"))
    {
        optionsOpen = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Camera Options"))
    {
        cameraOptionsOpen = true;
    }

    if (optionsOpen)
    {
        ImGui::Begin("Options", &optionsOpen, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Checkbox("Draw Background", &drawBackground);
        ImGui::BeginDisabled(drawBackground);
        ImGui::ColorEdit3("Clear Color", clearColor);
        ImGui::EndDisabled();
        ImGui::Separator();
        if (ImGui::Button("Post-Processing Stack"))
            postProcessOpen = true;
        ImGui::End();
    }

    if (postProcessOpen)
    {
        renderPostProcessStack();
    }

    if (cameraOptionsOpen)
    {
        ImGui::Begin("Camera Options", &cameraOptionsOpen, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Sensitivity");
        ImGui::Separator();
        ImGui::SliderFloat("Rotate Sensitivity", &InputProcesser::SENSITIVITY_ROTATE, 0.0f, 0.2f);
        ImGui::SliderFloat("Translate Sensitivity", &InputProcesser::SENSITIVITY_TRANSLATE, 0.0f, 0.2f);
        ImGui::SliderFloat("Zoom Sensitivity", &InputProcesser::SENSITIVITY_ZOOM, 0.0f, 0.2f);
        ImGui::SliderFloat("FOV Sensitivity", &InputProcesser::SENSITIVITY_FOV, 0.0f, 20.0f);
        // TODO: change FOV
//        ImGui::Text("Viewing");
//        ImGui::Separator();
        ImGui::End();
    }

    renderMenuBar();

//    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x, y), 0, ImVec2(1.0f, 0));
//    ImGui::Begin("Visual Options", nullptr, flags);
//    if (ImGui::Combo("Shader", &currentShaderIndex,
//                     "FXAA\0"))
//    {
//
//    }
//    ImGui::End();

}

void UserInterface::begin()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UserInterface::end(VkCommandBuffer commandBuffer)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void UserInterface::renderMenuBar()
{
    bool about = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::MenuItem("Quit"))
        {
            glfwSetWindowShouldClose(window->getWindow(), GLFW_TRUE);
        }

        if (ImGui::MenuItem("About"))
            about = true;

        if (ImGui::MenuItem("Load Model"))
        {
            IGFD::FileDialogConfig config;config.path = "../resources/gltf/";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".gltf,.glb", config);
        }

        float status = GLTFLoader::getLoadPercent();
        static float smoothPercent = 1.f;
        smoothPercent = glm::mix(smoothPercent, status, 0.2f);

        if (smoothPercent < 0.99f)
        {
            char ps[10];
            snprintf(ps, sizeof(ps), "%d", (int)(smoothPercent * 100));
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Status:");
            ImGui::ProgressBar(smoothPercent, ImVec2(100, 0), ps);
        }
        else
        {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Ready");
        }

        const char* name = "Aaron Tian 2024";
        ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(name).x - 15);
        ImGui::Text(name);
        ImGui::EndMainMenuBar();
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::cout << "File Path: " << filePathName << std::endl;
            // call a function passed from vulkan app
            modelSelectCallback(filePathName);
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }

    if (about)
        ImGui::OpenPopup("instruction_popup");

    if (ImGui::BeginPopup("instruction_popup"))
    {
        ImGui::Text("Instructions:");
        ImGui::BulletText("Mouse 1: Rotate Camera");
        ImGui::BulletText("Mouse 2: Translate Camera");
        ImGui::BulletText("Scroll Wheel: Zoom Camera");
        ImGui::BulletText("Mouse 4: Increase FOV");
        ImGui::BulletText("Mouse 5: Decrease FOV");
        ImGui::EndPopup();
    }
    ImGui::End();
}

void UserInterface::renderPostProcessStack()
{
    ImGui::Begin("Post-Processing Stack", &postProcessOpen, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Drag and drop effects to apply.");

    if (ImGui::BeginTable("table1", 2))
    {
        float columnWidth = 120.f;
        ImGui::TableSetupColumn("Available Effects", ImGuiTableColumnFlags_WidthFixed, columnWidth);
        ImGui::TableSetupColumn("Active Effects", ImGuiTableColumnFlags_WidthFixed, columnWidth);

        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        // list box
        static const char* items[] = { "FXAA", "Sharpen", "Chrom. Abbr.", "Vignette"};
        static int item_current_idx = 0;
        float listboxHeight = ImGui::GetTextLineHeightWithSpacing() * 10;
        if (ImGui::BeginListBox("##listbox 1", ImVec2(-FLT_MIN, listboxHeight)))
        {
            for (int n = 0; n < IM_ARRAYSIZE(items); n++)
            {
                ImGui::PushID(n);
                ImGui::Selectable(items[n]);

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    ImGui::SetDragDropPayload("DND", &n, sizeof(int));

                    ImGui::Text("%s", items[n]);
                    ImGui::EndDragDropSource();
                }

                ImGui::PopID();
            }
            ImGui::EndListBox();
        }
        ImGui::TableNextColumn();
        if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, listboxHeight)))
        {
            static char* current_item = NULL;
            ImGui::SetNextItemWidth(columnWidth * 0.95f);
            if (ImGui::BeginCombo("##combo", current_item))
            {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                {
                    bool is_selected = (current_item == items[n]);
                    ImGui::Selectable(items[n], is_selected);
                }
                ImGui::EndCombo();
            }
            // selectable, if selected then open menu for parameters, which also has button to remove
            ImGui::EndListBox();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                int payload_n = *(const int*)payload->Data;
                std::cout << "Dropped " << items[payload_n] << std::endl;
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
