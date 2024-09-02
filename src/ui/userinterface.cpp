#include "userinterface.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include "vulkan/vulkan.h"
#include <iterator>
#include <filesystem>
#include "../inputprocesser.h"
#include <iostream>
#include "ImGuiFileDialog-0.6.7/ImGuiFileDialog.h"
#include "../models/gltfloader.h"
#include "imgui_internal.h"

UserInterface::UserInterface() {}

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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;

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
    init_info.QueueFamily = param.context->queueManager.getGraphicsIndex();
    init_info.Queue = param.context->queueManager.queuesIndices.graphics;
    init_info.DescriptorPool = imguiPool;
    init_info.RenderPass = param.renderPass->getRenderPass();
    init_info.MinImageCount = param.framesInFlight;
    init_info.ImageCount = param.framesInFlight;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    window = param.window;

    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplVulkan_CreateFontsTexture();
}

void UserInterface::destroy()
{
    vkDeviceWaitIdle(device); // wait for operations to finish before disposing
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(device, imguiPool, nullptr); // must happen after ImGui_ImplVulkan_Shutdown
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

static ImGuiID dockLeftId, dockRightId;

void UserInterface::render(MenuPayloads menuPayloads)
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 5.0f;

    static bool firstTime = true;
    if (firstTime)
    {
        // Set up initial dock layout
        ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
        ImGui::DockBuilderRemoveNode(dockspaceId); // Clear out existing layout
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace); // Add back the dock space without any nodes
        ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

        // Split the dockspace to create a left dock area
        ImGuiID dockMainId = dockspaceId;
        ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.2f, &dockLeftId, &dockRightId);

        ImGui::DockBuilderDockWindow("Stats", dockLeftId);
        ImGui::DockBuilderDockWindow("Render", dockRightId);

        ImGui::DockBuilderFinish(dockspaceId);

        firstTime = false;
    }

//    ImGui::ShowDemoWindow();
    ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
    ImGui::DockSpaceOverViewport(dockspaceId, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGuiWindowClass windowClass;
    windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton
            | ImGuiDockNodeFlags_NoDockingOverOther;
    ImGui::SetNextWindowClass(&windowClass);

    ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoMove);

    if (menuPayloads.deviceName)
        ImGui::Text("Device: %s", menuPayloads.deviceName->c_str());
    else
        ImGui::Text("Device: N/A");

    ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);

    ImGui::Text("Frame Time: %f ms", ImGui::GetIO().DeltaTime * 1000.f);

    frameTimes.push_back(ImGui::GetIO().DeltaTime);
    if (frameTimes.size() > 100)
        frameTimes.erase(frameTimes.begin());

    ImGui::PlotLines("", frameTimes.data(), frameTimes.size(), 0, "",
                     0.f, 0.05f, ImVec2(ImGui::GetContentRegionAvail().x, 40));

    ImGui::Separator();
    if (ImGui::Button("Visual Options"))
    {
        visualMenu.open = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Camera Options"))
    {
        cameraMenu.open = true;
    }

    menuPayloads.visualMenuPayload.postProcessOpen = &postProcessMenu.open;
    menuPayloads.visualMenuPayload.lightsOpen = &lightMenu.open;
    visualMenu.renderMenu(&menuPayloads.visualMenuPayload);
    postProcessMenu.renderMenu(menuPayloads.postProcessManager);
    cameraMenu.renderMenu(menuPayloads.camera);
    lightMenu.renderMenu(&menuPayloads.lightsList);

    renderMenuBar();
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

        float status = GLTFLoader::getLoadPercent();
        ImGui::BeginDisabled(status < 1.f);
        if (ImGui::MenuItem("Load Model"))
        {
            IGFD::FileDialogConfig config;
            config.path = "../resources/gltf/";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".gltf,.glb", config);
        }
        ImGui::EndDisabled();

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
        ImGui::Text("%s", name);
        ImGui::Text("%s", name);
        ImGui::EndMainMenuBar();
    }

    // dont allow this window to dock
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
    {
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
        ImGui::BulletText("Ctrl +: Zoom in image");
        ImGui::BulletText("Ctrl -: Zoom out image");
        ImGui::BulletText("Middle Mouse: Pan image (double click to reset)");
        ImGui::BulletText("Alt + Left Mouse: Pan image (double click to reset)");
        ImGui::BulletText("ESC: Quit");
        ImGui::EndPopup();
    }
    ImGui::End();
}

bool UserInterface::renderImage(vk::DescriptorSet image, ImVec2 size)
{
    ImGuiWindowClass windowClass;
    windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoDockingSplitOther | ImGuiDockNodeFlags_NoWindowMenuButton;
    ImGui::SetNextWindowClass(&windowClass);

    ImGui::Begin("Render", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove);
    static float multiplier = 1.f;
    static ImVec2 offset = ImVec2(0, 0);
    bool isHovered = ImGui::IsWindowHovered();

    if (ImGui::IsWindowHovered())
    {
        auto trackpad = ImGui::IsKeyDown(ImGuiKey_LeftAlt) && ImGui::IsMouseDown(ImGuiMouseButton_Left);
        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) || trackpad)
        {
            offset.x += ImGui::GetIO().MouseDelta.x;
            offset.y += ImGui::GetIO().MouseDelta.y;
        }
        // double click to reset
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Middle) ||
            (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)))
        {
            offset = ImVec2(0, 0);
        }
        auto controlPlus = ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Equal);
        auto controlMinus = ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Minus);

        if (controlPlus)
        {
            multiplier += 0.1f;
            // TODO: zoom in on mouse cursor position
        }

        if (controlMinus)
            multiplier -= 0.1f;
    }
    ImGui::SetCursorPos(offset);
    ImGui::Image((ImTextureID)image, size * multiplier);
    ImGui::End();

    return isHovered;
}
