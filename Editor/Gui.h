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
#include <tuple>
#include "Actor.h"
#include "Project.h"
#include "RenderDevice.h"
#include "font-fk.h"
#include "font-roboto.h"

namespace UltraEd
{
    class Scene;

    class Gui
    {
    public:
        Gui(HWND hWnd);
        ~Gui();
        void Render();
        void Resize(UINT width, UINT height);
        ImGuiIO &IO();
        void OpenContextMenu(Actor *selectedActor);
        void ConfirmScene(std::function<void()> block);
        void RefreshScene(const std::vector<boost::uuids::uuid> &changedAssetIds);
    
    private:
        void PrepareFrame();
        void LoadFonts();
        void CustomStyle(ImGuiStyle* style);
        void DarkColors();
        void LightColors();
        void ThemeEditor();
        void LoadColorTheme();
        void ReleaseSceneTexture();
        void FileMenu();
        void EditMenu();
        void ActorMenu();
        void ViewMenu();
        void GizmoMenu();
        void Console();
        void KeyListener();
        void SceneGraph();
        void SceneView();
        void Properties();
        void ActorProperties();
        void OptionsModal();
        void SceneSettingsModal();
        void ContextMenu();
        void ScriptEditor();
        void NewProjectModal();
        void LoadProjectModal();
        void AddTextureModal();
        void AddModelModal();
        void LoadSceneModal();
        void SaveSceneModal();
        void ConfirmSceneModal();
        void SaveScene(bool openModal = false);
        void RenderTreeNode(Actor *actor);

    private:
        std::unique_ptr<Scene> m_scene;
        HWND m_hWnd;
        RenderDevice m_renderDevice;
        LPDIRECT3DTEXTURE9 m_sceneTexture;
        LPDIRECT3DTEXTURE9 m_noTexture;
        Actor *m_selectedActor;
        TextEditor m_textEditor;
        ImGui::FileBrowser m_fileBrowser;
        ImGui::FileBrowser m_folderBrowser;
        std::string m_consoleText;
        bool m_moveConsoleToBottom;
        bool m_openContextMenu;
        std::tuple<bool, std::function<void()>> m_saveSceneModalOpen;
        std::tuple<bool, std::function<void()>> m_openConfirmSceneModal;
        bool m_textEditorOpen;
        bool m_optionsModalOpen;
        bool m_sceneSettingsModalOpen;
        bool m_newProjectModalOpen;
        bool m_loadProjectModalOpen;
        bool m_addTextureModalOpen;
        bool m_addModelModalOpen;
        bool m_loadSceneModalOpen;
    };
}

#endif
