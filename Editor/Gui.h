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
#include "ModalProperties.h"
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
        void ConfirmScene(std::function<void()> onComplete);
        void RefreshScene(const std::vector<boost::uuids::uuid> &changedAssetIds);
        void LoadProject(const std::filesystem::path &path);
        static const int ImageButtonWidth = 64;
    
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
        void HandleROMBuilding();
        void EditMenu();
        void ActorMenu();
        void ViewMenu();
        void GizmoMenu();
        void Console();
        void KeyListener();
        void SceneGraph();
        void RevealSelectedActorNode(ImGuiID stackID);
        void SceneGraphMenuBar(const std::vector<Actor *> &actors, ImGuiID stackID);
        void SceneView();
        void Properties();
        void SetActorName(Actor *targetActor, const boost::uuids::uuid &groupId);
        void SetActorPosition(Actor *targetActor, const boost::uuids::uuid &groupId);
        void SetActorRotation(Actor *targetActor, const boost::uuids::uuid &groupId);
        void SetActorScale(Actor *targetActor, const boost::uuids::uuid &groupId);
        void ShowModelProperties(Actor *targetActor);
        void ShowCameraProperties(Actor *targetActor);
        void OptionsModal();
        void SceneSettingsModal();
        void ContextMenu();
        void ScriptEditor();
        void SaveScriptEditor(Actor *actor = nullptr);
        void NewProjectModal();
        void LoadProjectModal();
        void AddTextureModal();
        void AddModelModal();
        void LoadSceneModal();
        void SaveSceneModal();
        void ConfirmModal();
        void SaveScene(bool openModal = false);
        void RenderTreeNode(Actor *actor, ImGuiID stackID);
        void RenderTreeNodeChildren(bool isOpen, Actor *actor, ImGuiID stackID);
        void HandleTreeNodeContextMenu(Actor *actor);
        void HandleTreeNodeDragDrop(Actor *actor);
        void StatusBar();

    private:
        std::unique_ptr<Scene> m_scene;
        HWND m_hWnd;
        RenderDevice m_renderDevice;
        LPDIRECT3DTEXTURE9 m_sceneTexture;
        LPDIRECT3DTEXTURE9 m_noTexture;
        Actor *m_selectedActor;
        Actor *m_recentSelectedActor;
        std::map<Actor *, std::tuple<std::string, std::shared_ptr<TextEditor>>> m_scriptEditors;
        ImGuiID m_scriptEditorDockTargetID;
        ImGui::FileBrowser m_fileBrowser;
        ImGui::FileBrowser m_folderBrowser;
        std::string m_consoleText;
        ModalProperties m_saveSceneModal;
        ModalProperties m_openConfirmModal;
        bool m_moveConsoleToBottom;
        bool m_openContextMenu;
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
