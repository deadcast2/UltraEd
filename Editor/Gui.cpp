#include <ImGui/imconfig.h>
#include "Gui.h"
#include "FileIO.h"
#include "PubSub.h"
#include "Scene.h"
#include "Settings.h"
#include "View.h"

namespace UltraEd
{
    Gui::Gui(Scene *scene, HWND hWnd, IDirect3DDevice9 *device) :
        m_scene(scene),
        m_buildOutput(),
        m_moveBuildOutputToBottom(false),
        m_selectedActorIndex(0),
        m_optionsModelOpen(false)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(hWnd);
        ImGui_ImplDX9_Init(device);

        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        PubSub::Subscribe({ "BuildOutputClear", [&](void *data) {
            m_buildOutput.clear();
        } });

        PubSub::Subscribe({ "BuildOutputAppend", [&](void *data) {
            auto text = static_cast<char *>(data);
            m_buildOutput.append(text);
            m_moveBuildOutputToBottom = true;
        } });
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

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowBgAlpha(0.0f);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", 0, window_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("RootDockspace");
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        if (ImGui::BeginMainMenuBar())
        {
            FileMenu();
            ActorMenu();
            ViewMenu();
            GizmoMenu();
            ImGui::EndMainMenuBar();
        }

        SceneGraph();
        BuildOutput();
        OptionsModal();

        //ImGui::ShowDemoWindow();

        ImGui::End();
        ImGui::EndFrame();
    }

    void Gui::RenderFrame()
    {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    ImGuiIO &Gui::IO()
    {
        return ImGui::GetIO();
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

            if (ImGui::MenuItem("Build ROM"))
            {
                m_scene->OnBuildROM(BuildFlag::Build);
            }

            if (ImGui::MenuItem("Build ROM & Load"))
            {
                m_scene->OnBuildROM(BuildFlag::Load);
            }

            if (ImGui::MenuItem("Build ROM & Run"))
            {
                m_scene->OnBuildROM(BuildFlag::Run);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Install Build Tools"))
            {
                char pathBuffer[128];
                GetFullPathName("..\\Engine\\tools.bin", 128, pathBuffer, NULL);
                if (FileIO::Unpack(pathBuffer))
                {
                    MessageBox(0, "Build tools successfully installed.", "Success!", MB_OK);
                }
                else
                {
                    MessageBox(0, "Could not find build tools.", "Error", MB_OK);
                }
            }

            if (ImGui::MenuItem("Options"))
            {
                m_optionsModelOpen = true;
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

            ImGui::Separator();

            if (ImGui::MenuItem("Go To", "F"))
            {
                m_scene->FocusSelected();
            }

            if (ImGui::MenuItem("Home", "H"))
            {
                m_scene->ResetViews();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Solid", 0, m_scene->m_fillMode == D3DFILL_SOLID))
            {
                m_scene->ToggleFillMode();
            }

            ImGui::EndMenu();
        }
    }

    void Gui::GizmoMenu()
    {
        if (ImGui::BeginMenu("Gizmo"))
        {
            if (ImGui::MenuItem("Translate", "1", m_scene->m_gizmo.m_modifierState == GizmoModifierState::Translate))
            {
                m_scene->SetGizmoModifier(GizmoModifierState::Translate);
            }

            if (ImGui::MenuItem("Rotate", "2", m_scene->m_gizmo.m_modifierState == GizmoModifierState::Rotate))
            {
                m_scene->SetGizmoModifier(GizmoModifierState::Rotate);
            }

            if (ImGui::MenuItem("Scale", "3", m_scene->m_gizmo.m_modifierState == GizmoModifierState::Scale))
            {
                m_scene->SetGizmoModifier(GizmoModifierState::Scale);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("World Space", 0, m_scene->m_gizmo.m_worldSpaceToggled))
            {
                m_scene->ToggleMovementSpace();
            }

            if (ImGui::MenuItem("Snap to Grid", 0, m_scene->m_gizmo.m_snapToGridToggled))
            {
                m_scene->ToggleSnapToGrid();
            }

            ImGui::EndMenu();
        }
    }

    void Gui::SceneGraph()
    {
        if (ImGui::Begin("Scene Graph", 0, ImGuiWindowFlags_HorizontalScrollbar))
        {
            ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
                | ImGuiTreeNodeFlags_SpanAvailWidth;

            auto actors = m_scene->GetActors();
            for (int i = 0; i < actors.size(); i++)
            {
                ImGuiTreeNodeFlags leafFlags = baseFlags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (m_selectedActorIndex == i)
                    leafFlags |= ImGuiTreeNodeFlags_Selected;

                ImGui::TreeNodeEx((void *)(intptr_t)i, leafFlags, actors[i]->GetName().c_str());
                if (ImGui::IsItemClicked())
                {
                    m_scene->SelectActorById(actors[i]->GetId());
                    m_selectedActorIndex = i;
                }
            }
        }

        ImGui::End();
    }

    void Gui::BuildOutput()
    {
        if (ImGui::Begin("Build Output", 0, ImGuiWindowFlags_HorizontalScrollbar))
        {
            ImGui::TextUnformatted(m_buildOutput.c_str());
            if (m_moveBuildOutputToBottom)
            {
                ImGui::SetScrollHereY(1);
                m_moveBuildOutputToBottom = false;
            }
        }

        ImGui::End();
    }

    void Gui::OptionsModal()
    {
        static int videoMode;
        static int buildCart;

        if (m_optionsModelOpen)
        {
            ImGui::OpenPopup("Options");

            videoMode = static_cast<int>(Settings::GetVideoMode());
            buildCart = static_cast<int>(Settings::GetBuildCart());

            m_optionsModelOpen = false;
        }

        if (ImGui::BeginPopupModal("Options", 0, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Combo("Video Mode", &videoMode, "NTSC\0PAL\0\0");
            ImGui::Combo("Build Cart", &buildCart, "64drive\0EverDrive-64 X7\0\0");
            
            if (ImGui::Button("Save"))
            {
                Settings::SetVideoMode(static_cast<VideoMode>(videoMode));
                Settings::SetBuildCart(static_cast<BuildCart>(buildCart));
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
}
