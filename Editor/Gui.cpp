#include <ImGui/imconfig.h>
#include <thread>
#include "Debug.h"
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
        m_consoleText(),
        m_moveConsoleToBottom(false),
        m_openContextMenu(false),
        m_textEditorOpen(false),
        m_optionsModalOpen(false),
        m_sceneSettingsModalOpen(false),
        m_selectedActor()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        LoadColorTheme();
        ImGui_ImplWin32_Init(hWnd);
        ImGui_ImplDX9_Init(device);

        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::C());

        PubSub::Subscribe({ "AppendToConsole", [&](void *data) {
            auto text = static_cast<std::string *>(data);
            m_consoleText.append(*text);
            m_moveConsoleToBottom = true;
        } });

        PubSub::Subscribe({ "ContextMenu", [&](void *data) {
            m_openContextMenu = true;
            m_selectedActor = static_cast<Actor *>(data);
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
        {
            ImGui::PopStyleVar(3);

            ImGuiID dockspace_id = ImGui::GetID("RootDockspace");
            ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

            if (ImGui::BeginMainMenuBar())
            {
                FileMenu();
                EditMenu();
                ActorMenu();
                ViewMenu();
                GizmoMenu();
                ImGui::EndMainMenuBar();
            }

            SceneGraph();
            Console();
            ActorProperties();
            OptionsModal();
            SceneSettingsModal();
            ContextMenu();
            ScriptEditor();
            //ImGui::ShowDemoWindow();
        }
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

    void Gui::RebuildWith(std::function<void()> inner)
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();

        if (inner) inner();

        ImGui_ImplDX9_CreateDeviceObjects();
    }

    void Gui::LoadColorTheme()
    {
        switch (Settings::GetColorTheme())
        {
            case ColorTheme::Classic:
                ImGui::StyleColorsClassic();
                break;
            case ColorTheme::Dark:
                ImGui::StyleColorsDark();
                break;
            case ColorTheme::Light:
                ImGui::StyleColorsLight();
                break;
        }
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
                Debug::Info("Installing build tools...");

                std::thread run([pathBuffer]() {
                    if (FileIO::Unpack(pathBuffer))
                    {
                        Debug::Info("Build tools successfully installed.");
                    }
                    else
                    {
                        Debug::Error("Could not find build tools.");
                    }
                });
                run.detach();
            }

            if (ImGui::MenuItem("Options"))
            {
                m_optionsModalOpen = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                PubSub::Publish("Exit");
            }

            ImGui::EndMenu();
        }
    }

    void Gui::EditMenu()
    {
        if (ImGui::BeginMenu("Edit"))
        {
            auto auditorTitles = m_scene->m_auditor.Titles();

            if (ImGui::MenuItem(auditorTitles[0].c_str(), "Ctrl+Z"))
            {
                m_scene->m_auditor.Undo();
            }

            if (ImGui::MenuItem(auditorTitles[1].c_str(), "Ctrl+Y"))
            {
                m_scene->m_auditor.Redo();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Duplicate", "Ctrl+D"))
            {
                m_scene->Duplicate();
            }

            if (ImGui::MenuItem("Delete", "Delete"))
            {
                m_scene->Delete();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Select All", "Ctrl+A"))
            {
                m_scene->SelectAll();
            }

            if (ImGui::MenuItem("Scene Settings"))
            {
                m_sceneSettingsModalOpen = true;
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
                m_scene->m_gizmo.SetModifier(GizmoModifierState::Translate);
            }

            if (ImGui::MenuItem("Rotate", "2", m_scene->m_gizmo.m_modifierState == GizmoModifierState::Rotate))
            {
                m_scene->m_gizmo.SetModifier(GizmoModifierState::Rotate);
            }

            if (ImGui::MenuItem("Scale", "3", m_scene->m_gizmo.m_modifierState == GizmoModifierState::Scale))
            {
                m_scene->m_gizmo.SetModifier(GizmoModifierState::Scale);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("World Space", 0, m_scene->m_gizmo.m_worldSpaceToggled))
            {
                m_scene->ToggleMovementSpace();
            }

            if (ImGui::MenuItem("Snap to Grid", 0, m_scene->m_gizmo.m_snapToGridToggled))
            {
                m_scene->m_gizmo.ToggleSnapping();
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
                if (m_scene->IsActorSelected(actors[i]->GetId()))
                    leafFlags |= ImGuiTreeNodeFlags_Selected;

                ImGui::TreeNodeEx((void *)(intptr_t)i, leafFlags, actors[i]->GetName().c_str());
                if (ImGui::IsItemClicked())
                    m_scene->SelectActorById(actors[i]->GetId(), !IO().KeyShift);
            }
        }

        ImGui::End();
    }

    void Gui::Console()
    {
        if (ImGui::Begin("Console", 0, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar))
        {
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::SmallButton("Clear"))
                {
                    m_consoleText.clear();
                }

                ImGui::EndMenuBar();
            }

            ImGui::TextUnformatted(m_consoleText.c_str());
            if (m_moveConsoleToBottom)
            {
                ImGui::SetScrollHereY(1);
                m_moveConsoleToBottom = false;
            }
        }

        ImGui::End();
    }

    void Gui::ActorProperties()
    {
        if (ImGui::Begin("Properties", 0, ImGuiWindowFlags_HorizontalScrollbar))
        {
            char name[100] = { 0 };
            float position[3] = { 0 };
            float rotation[3] = { 0 };
            float scale[3] = { 0 };
            auto actors = m_scene->GetActors(true);
            Actor *targetActor = NULL;

            if (actors.size() > 0)
            {
                // Show the properties of the last selected actor.
                targetActor = actors[actors.size() - 1];
                sprintf(name, targetActor->GetName().c_str());
                Util::ToFloat3(targetActor->GetPosition(), position);
                Util::ToFloat3(targetActor->GetRotation(), rotation);
                Util::ToFloat3(targetActor->GetScale(), scale);
            }

            char tempName[100];
            sprintf(tempName, name);
            ImGui::InputText("Name", name, 100);

            D3DXVECTOR3 tempPos = D3DXVECTOR3(position);
            ImGui::InputFloat3("Position", position, "%g");

            D3DXVECTOR3 tempRot = D3DXVECTOR3(rotation);
            ImGui::InputFloat3("Rotation", rotation, "%g");

            D3DXVECTOR3 tempScale = D3DXVECTOR3(scale);
            ImGui::InputFloat3("Scale", scale, "%g");

            GUID groupId = Util::NewGuid();

            // Only apply changes to selected actors when one if it's properties has changed
            // to make all actors mutate as expected.
            for (int i = 0; i < actors.size(); i++)
            {
                if (strcmp(tempName, name) != 0)
                {
                    m_scene->m_auditor.ChangeActor("Name Set", actors[i]->GetId(), groupId);
                    actors[i]->SetName(std::string(name));
                }

                if (tempPos != D3DXVECTOR3(position))
                {
                    auto curPos = actors[i]->GetPosition();
                    m_scene->m_auditor.ChangeActor("Position Set", actors[i]->GetId(), groupId);
                    actors[i]->SetPosition(D3DXVECTOR3(
                        tempPos.x != position[0] ? position[0] : curPos.x,
                        tempPos.y != position[1] ? position[1] : curPos.y,
                        tempPos.z != position[2] ? position[2] : curPos.z
                    ));

                    // Make sure the gizmo follows the target actor.
                    if (actors[i] == targetActor)
                        m_scene->m_gizmo.Update(targetActor);
                }

                if (tempRot != D3DXVECTOR3(rotation))
                {
                    auto curRot = actors[i]->GetRotation();
                    m_scene->m_auditor.ChangeActor("Rotation Set", actors[i]->GetId(), groupId);
                    actors[i]->SetRotation(D3DXVECTOR3(
                        tempRot.x != rotation[0] ? rotation[0] : curRot.x,
                        tempRot.y != rotation[1] ? rotation[1] : curRot.y,
                        tempRot.z != rotation[2] ? rotation[2] : curRot.z
                    ));
                }

                if (tempScale != D3DXVECTOR3(scale))
                {
                    auto curScale = actors[i]->GetScale();
                    m_scene->m_auditor.ChangeActor("Scale Set", actors[i]->GetId(), groupId);
                    actors[i]->SetScale(D3DXVECTOR3(
                        tempScale.x != scale[0] ? scale[0] : curScale.x,
                        tempScale.y != scale[1] ? scale[1] : curScale.y,
                        tempScale.z != scale[2] ? scale[2] : curScale.z
                    ));
                }
            }
        }

        ImGui::End();
    }

    void Gui::OptionsModal()
    {
        static int videoMode;
        static int buildCart;
        static int colorTheme;

        if (m_optionsModalOpen)
        {
            ImGui::OpenPopup("Options");

            videoMode = static_cast<int>(Settings::GetVideoMode());
            buildCart = static_cast<int>(Settings::GetBuildCart());
            colorTheme = static_cast<int>(Settings::GetColorTheme());

            m_optionsModalOpen = false;
        }

        if (ImGui::BeginPopupModal("Options", 0, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Combo("Color Theme", &colorTheme, "Classic\0Dark\0Light\0\0");
            ImGui::Combo("Video Mode", &videoMode, "NTSC\0PAL\0\0");
            ImGui::Combo("Build Cart", &buildCart, "64drive\0EverDrive-64 X7\0\0");

            if (ImGui::Button("Save"))
            {
                Settings::SetColorTheme(static_cast<ColorTheme>(colorTheme));
                LoadColorTheme();

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

    void Gui::SceneSettingsModal()
    {
        static float backgroundColor[3];
        static float gridSnapSize;

        if (m_sceneSettingsModalOpen)
        {
            ImGui::OpenPopup("Scene Settings");

            backgroundColor[0] = m_scene->m_backgroundColorRGB[0] / 255.0f;
            backgroundColor[1] = m_scene->m_backgroundColorRGB[1] / 255.0f;
            backgroundColor[2] = m_scene->m_backgroundColorRGB[2] / 255.0f;
            gridSnapSize = m_scene->m_gizmo.GetSnapSize();

            m_sceneSettingsModalOpen = false;
        }

        if (ImGui::BeginPopupModal("Scene Settings", 0, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::ColorEdit3("Background Color", backgroundColor);
            ImGui::InputFloat("Grid Snap Size", &gridSnapSize);

            if (ImGui::Button("Save"))
            {
                m_scene->SetBackgroundColor(RGB(backgroundColor[0] * 255, backgroundColor[1] * 255,
                    backgroundColor[2] * 255));
                m_scene->SetGizmoSnapSize(gridSnapSize);

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

    void Gui::ContextMenu()
    {
        if (m_openContextMenu)
        {
            ImGui::OpenPopup("Context Menu");
            m_openContextMenu = false;
        }

        if (ImGui::BeginPopup("Context Menu"))
        {
            if (ImGui::MenuItem("Edit Script"))
            {
                m_textEditorOpen = true;
                m_textEditor.SetText(m_scene->GetScript());
            }

            if (ImGui::BeginMenu("Texture"))
            {
                if (ImGui::MenuItem("Add"))
                {
                    m_scene->OnAddTexture();
                }

                if (m_selectedActor != NULL && reinterpret_cast<Model *>(m_selectedActor)->HasTexture())
                {
                    ImGui::Separator();

                    if (ImGui::MenuItem("Delete"))
                    {
                        m_scene->OnDeleteTexture();
                    }
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

                if (m_selectedActor != NULL && m_selectedActor->HasCollider())
                {
                    ImGui::Separator();

                    if (ImGui::MenuItem("Delete"))
                    {
                        m_scene->OnDeleteCollider();
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Delete"))
            {
                m_scene->Delete();
            }

            if (ImGui::MenuItem("Duplicate"))
            {
                m_scene->Duplicate();
            }

            ImGui::EndPopup();
        }
    }

    void Gui::ScriptEditor()
    {
        if (!m_textEditorOpen)
            return;

        ImGui::Begin("Script Editor", &m_textEditorOpen, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save Changes"))
                {
                    m_scene->SetScript(m_textEditor.GetText());
                }

                if (ImGui::MenuItem("Close"))
                {
                    m_textEditorOpen = false;
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        m_textEditor.Render("Edit Script");

        ImGui::End();
    }
}
