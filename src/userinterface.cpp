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

UserInterface::UserInterface() {}

UserInterface::~UserInterface()
{
    vkDeviceWaitIdle(device); // wait for operations to finish before disposing
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(device, imguiPool, nullptr); // must happen after ImGui_ImplVulkan_Shutdown
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UserInterface::create(ImGuiCreateParameters param, CommandPool commandPool)
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
    commandPool.submitSingleTimeCommands(param.context->queueManager, {commandBuffer},
                                         vkb::QueueType::graphics, true);
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
    const int y = 21;
    //ImGui::ShowDemoWindow();
    ImGui::SetNextWindowPos(ImVec2(0, y));
    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

    ImGui::Begin("Stats", nullptr, flags);

    ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);

    ImGui::Text("Frame Time: %f ms", ImGui::GetIO().DeltaTime * .001f);

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
        ImGui::ColorEdit3("Clear Color", clearColor);
        ImGui::End();
    }

    if (cameraOptionsOpen)
    {
        ImGui::Begin("Camera Options", &cameraOptionsOpen, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SliderFloat("Rotate Sensitivity", &InputProcesser::SENSITIVITY_ROTATE, 0.0f, 0.2f);
        ImGui::SliderFloat("Translate Sensitivity", &InputProcesser::SENSITIVITY_TRANSLATE, 0.0f, 0.2f);
        ImGui::SliderFloat("Zoom Sensitivity", &InputProcesser::SENSITIVITY_ZOOM, 0.0f, 0.2f);
        ImGui::End();
    }

    renderMenuBar();

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x, y), 0, ImVec2(1.0f, 0));
    ImGui::Begin("Visual Options", nullptr, flags);
    if (ImGui::Combo("Shader", &currentShaderIndex,
                     "FXAA\0"))
    {

    }

    ImGui::End();
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

        auto status = GLTFLoader::getLoadStatus();

        ImVec4 textColor;
        std::string statusText = "Status: ";
        switch(status)
        {
            case LoadStatus::READY:
                statusText += "Ready";
                textColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                break;
            case LoadStatus::PARSING:
                statusText += "Parsing File...";
                textColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                break;
            case LoadStatus::LOADING:
                statusText += "Loading Nodes, Materials...";
                textColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                break;
            case LoadStatus::LOADED_MAT_TEX:
                statusText += "Loading Nodes...";
                textColor = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
                break;
            case LoadStatus::LOADED_NODE:
                statusText += "Loading Materials and Textures...";
                textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
        }

        auto cString = statusText.c_str();
        ImGui::TextColored(textColor, cString);

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
