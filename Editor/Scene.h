#ifndef _SCENE_H_
#define _SCENE_H_

#include <array>
#include <map>
#include <d3d9.h>
#include <d3dx9.h>
#include <nlohmann/json.hpp>
#include "Gui.h"
#include "View.h"
#include "Common.h"
#include "Gizmo.h"
#include "Grid.h"
#include "Model.h"
#include "Camera.h"
#include "Auditor.h"
#include "RenderDevice.h"

namespace UltraEd
{
    enum class BuildFlag
    {
        Build, Run, Load
    };

    class Scene : public Savable
    {
        friend Gui;

    public:
        Scene(Gui *gui);
        ~Scene();
        std::vector<Actor *> GetActors(bool selectedOnly = false);
        COLORREF GetBackgroundColor();
        HWND GetWndHandle();
        void Render(LPDIRECT3DDEVICE9 target, LPDIRECT3DTEXTURE9 *texture);
        nlohmann::json Save();
        nlohmann::json PartialSave();
        void PartialLoad(const nlohmann::json &root);
        void UnselectAll();
        std::shared_ptr<Actor> GetActor(const boost::uuids::uuid &id);
        void RestoreActor(const nlohmann::json &item);
        void Delete(std::shared_ptr<Actor> actor);
        void SelectActorById(const boost::uuids::uuid &id, bool clearAll = true);
        void Resize(int width, int height);
        void Refresh(const std::vector<boost::uuids::uuid> &changedAssetIds);
     
    private:
        void Delete();
        void Duplicate();
        void FocusSelected();
        void SetScript(std::string script);
        std::string GetScript();
        void SetBackgroundColor(COLORREF color);
        void SetGizmoSnapSize(float size);
        void New();
        bool Save(const std::filesystem::path &path);
        void Load(const std::filesystem::path &path);
        void AddCamera();
        void AddTexture(const boost::uuids::uuid &assetId);
        void DeleteTexture();
        void AddModel(const boost::uuids::uuid &assetId);
        void AddCollider(ColliderType type);
        void DeleteCollider();
        void BuildROM(BuildFlag flag);
        bool Pick(ImVec2 mousePoint, bool ignoreGizmo = false, Actor **selectedActor = NULL);
        void ReleaseResources(ModelRelease type);
        void ScreenRaycast(ImVec2 screenPoint, D3DXVECTOR3 *origin, D3DXVECTOR3 *dir);
        void SetViewType(ViewType type);
        View *GetActiveView();
        bool ToggleMovementSpace();
        void SelectAll();
        void Load(const nlohmann::json &root);
        void SetDirty(bool value);
        void ResetViews();
        std::string GetStats();
        bool ToggleFillMode();
        void CheckChanges();
        void CheckInput();
        void SetTitle(std::string title, bool store = true);
        void UpdateViewMatrix();
        void WrapCursor();
        bool IsActorSelected(const boost::uuids::uuid &id);

    private:
        D3DMATERIAL9 m_defaultMaterial;
        D3DFILLMODE m_fillMode;
        Gizmo m_gizmo;
        std::array<View, 4> m_views;
        std::map<boost::uuids::uuid, std::shared_ptr<Actor>> m_actors;
        Grid m_grid;
        std::vector<boost::uuids::uuid> m_selectedActorIds;
        float m_mouseSmoothX, m_mouseSmoothY;
        ViewType m_activeViewType;
        std::string m_sceneName;
        std::array<int, 3> m_backgroundColorRGB;
        Auditor m_auditor;
        Gui *m_gui;
        RenderDevice m_renderDevice;
    };
}

#endif
