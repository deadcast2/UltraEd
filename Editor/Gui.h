#ifndef _GUI_H_
#define _GUI_H_

#include <windows.h>
#include <functional>
#include <memory>
#include <string>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx9.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/Plugins/TextEditor.h>
#include <ImGui/Plugins/imfilebrowser.h>
#include "Actor.h"
#include "Project.h"

namespace UltraEd
{
    class Scene;

    class Gui
    {
    public:
        Gui(Scene *scene, HWND hWnd, IDirect3DDevice9 *device);
        ~Gui();
        void PrepareFrame();
        void RenderFrame();
        ImGuiIO &IO();
        void RebuildWith(std::function<void()> inner);

    private:
        void LoadColorTheme();
        void FileMenu();
        void EditMenu();
        void ActorMenu();
        void ViewMenu();
        void GizmoMenu();
        void Console();
        void SceneGraph();
        void ActorProperties();
        void OptionsModal();
        void SceneSettingsModal();
        void ContextMenu();
        void ScriptEditor();
        void NewProjectModal();

    private:
        Scene *m_scene;
        Actor *m_selectedActor;
        std::unique_ptr<Project> m_project;
        TextEditor m_textEditor;
        ImGui::FileBrowser m_fileBrowser;
        std::string m_consoleText;
        bool m_moveConsoleToBottom;
        bool m_openContextMenu;
        bool m_textEditorOpen;
        int m_optionsModalOpen;
        int m_sceneSettingsModalOpen;
        int m_newProjectModalOpen;
    };
}

#endif
