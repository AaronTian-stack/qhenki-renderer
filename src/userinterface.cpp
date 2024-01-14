#include "userinterface.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include <vulkan/vulkan.h>
#include <iterator>

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
    init_info.Instance = param.context->instance;
    init_info.PhysicalDevice = param.context->device.physicalDevice;
    init_info.Device = param.context->device.logicalDevice;
    init_info.Queue = param.context->queueManager.graphicsQueue;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = param.framesInFlight;
    init_info.ImageCount = param.framesInFlight;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    window = param.window;

    ImGui_ImplVulkan_Init(&init_info, param.renderPass->getRenderPass());

    auto commandBuffer = commandPool.beginSingleCommand();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    commandPool.endSingleTimeCommands(param.context->queueManager, commandBuffer);
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
    //ImGui::ShowDemoWindow();
    ImGui::SetNextWindowPos(ImVec2(0, 18));
    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

    ImGui::Begin("Pathtracer", nullptr, flags);

    ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);

    ImGui::Text("Frame Time: %f ms", ImGui::GetIO().DeltaTime);

    ImGui::Separator();
    if (ImGui::Button("Options"))
    {
        optionsOpen = true;
    }

    if (optionsOpen)
    {
        ImGui::Begin("Options", &optionsOpen, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::ColorEdit3("Clear Color", clearColor);
        ImGui::End();
    }

    renderMenuBar();

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x, 18), 0, ImVec2(1.0f, 0));
    ImGui::Begin("Visual Options", nullptr, flags);
    if (ImGui::Combo("Shader", &currentShaderIndex,
                     "Pathtrace\0Triangle\0"))
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

        // hack to get the menu to the right
        ImGui::SameLine(ImGui::GetWindowWidth() - 126);
        ImGui::Text("Aaron Tian 2024");
        ImGui::EndMainMenuBar();
    }

    if (about)
        ImGui::OpenPopup("instruction_popup");

    if (ImGui::BeginPopup("instruction_popup"))
    {
        ImGui::Text("lorem ipsum");
        ImGui::BulletText("todo");
        ImGui::EndPopup();
    }
    ImGui::End();
}
