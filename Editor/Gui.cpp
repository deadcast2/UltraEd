#include "Gui.h"
#include "PubSub.h"
#include "Scene.h"
#include "View.h"

namespace UltraEd
{
    Gui::Gui(Scene *scene, HWND hWnd, IDirect3DDevice9 *device) : 
        m_scene(scene)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(hWnd);
        ImGui_ImplDX9_Init(device);
    }

    Gui::~Gui()
    {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void Gui::PrepareFrame()
    {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar())
        {
            FileMenu();
            ActorMenu();
            RenderMenu();
            ViewMenu();

            ImGui::EndMainMenuBar();
        }

        ImGui::EndFrame();
    }

    void Gui::RenderFrame()
    {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    bool Gui::WantsMouse()
    {
        return ImGui::GetIO().WantCaptureMouse;
    }

    void Gui::RebuildWith(function<void()> inner)
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();

        if (inner) inner();

        ImGui_ImplDX9_CreateDeviceObjects();
    }

    void Gui::FileMenu()
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene"))
            {
                m_scene->OnNew();
            }

            if (ImGui::MenuItem("Save Scene As..."))
            {
                m_scene->OnSave();
            }

            if (ImGui::MenuItem("Load Scene"))
            {
                m_scene->OnLoad();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                PubSub::Publish("Exit");
            }

            ImGui::EndMenu();
        }
    }

    void Gui::ActorMenu()
    {
        if (ImGui::BeginMenu("Actor"))
        {
            if (ImGui::MenuItem("Camera"))
            {
                m_scene->OnAddCamera();
            }

            if (ImGui::MenuItem("Model"))
            {
                m_scene->OnAddModel(ModelPreset::Custom);
            }

            if (ImGui::BeginMenu("Texture"))
            {
                if (ImGui::MenuItem("Add"))
                {
                    m_scene->OnAddTexture();
                }

                if (ImGui::MenuItem("Delete"))
                {
                    m_scene->OnDeleteTexture();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Collider"))
            {
                if (ImGui::MenuItem("Box"))
                {
                    m_scene->OnAddCollider(ColliderType::Box);
                }

                if (ImGui::MenuItem("Sphere"))
                {
                    m_scene->OnAddCollider(ColliderType::Sphere);
                }

                if (ImGui::MenuItem("Delete"))
                {
                    m_scene->OnDeleteCollider();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Presets"))
            {
                if (ImGui::MenuItem("Pumpkin"))
                {
                    m_scene->OnAddModel(ModelPreset::Pumpkin);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
    }

    void Gui::RenderMenu()
    {
        if (ImGui::BeginMenu("Render"))
        {
            if (ImGui::MenuItem("Solid", 0, m_scene->IsSolidRender()))
            {
                m_scene->ToggleFillMode();
            }

            ImGui::EndMenu();
        }
    }

    void Gui::ViewMenu()
    {
        if (ImGui::BeginMenu("View"))
        {
            auto viewType = m_scene->GetActiveView()->GetType();

            if (ImGui::MenuItem("Perspective", 0, viewType == ViewType::Perspective))
            {
                m_scene->SetViewType(ViewType::Perspective);
            }

            if (ImGui::MenuItem("Top", 0, viewType == ViewType::Top))
            {
                m_scene->SetViewType(ViewType::Top);
            }

            if (ImGui::MenuItem("Left", 0, viewType == ViewType::Left))
            {
                m_scene->SetViewType(ViewType::Left);
            }

            if (ImGui::MenuItem("Front", 0, viewType == ViewType::Front))
            {
                m_scene->SetViewType(ViewType::Front);
            }

            ImGui::EndMenu();
        }
    }
}
