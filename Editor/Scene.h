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
        Scene(HWND hWnd, Gui *gui);
        ~Scene();
        std::vector<Actor *> GetActors(bool selectedOnly = false);
        Actor *GetActor(const boost::uuids::uuid &id);
        COLORREF GetBackgroundColor();
        void UpdateInput(const D3DXVECTOR2 &mousePos);
        void Render(LPDIRECT3DDEVICE9 target, LPDIRECT3DTEXTURE9 *texture);
        nlohmann::json Save();
        nlohmann::json PartialSave();
        void PartialLoad(const nlohmann::json &root);
        void UnselectAll();
        void RestoreActor(const nlohmann::json &item, bool markSceneDirty = false);
        void Delete(Actor *actor);
        void SelectActorById(const boost::uuids::uuid &id, bool clearAll = true);
        void Resize(UINT width, UINT height);
        void Refresh(const std::vector<boost::uuids::uuid> &changedAssetIds);
        bool HasPath();
        bool IsDragging() { return m_isDragging; }
        bool IsSelecting() { return m_isSelecting; }
        void SetModifier(GizmoModifierState state);
     
    private:
        void Delete();
        void Duplicate();
        void FocusSelected();
        void SetScript(std::string script);
        std::string GetScript();
        void SetBackgroundColor(COLORREF color);
        void SetGizmoSnapSize(float size);
        void New();
        bool SaveAs();
        bool SaveAs(const std::filesystem::path &path);
        void Load(const std::filesystem::path &path);
        void AddCamera();
        void AddTexture(const boost::uuids::uuid &assetId);
        void DeleteTexture();
        void AddModel(const boost::uuids::uuid &assetId);
        void AddCollider(ColliderType type);
        void DeleteCollider();
        void BuildROM(BuildFlag flag);
        bool Pick(const D3DXVECTOR2 &mousePoint, bool ignoreGizmo = false, Actor **selectedActor = NULL);
        void Release();
        void SetViewType(ViewType type);
        View *GetActiveView();
        bool ToggleMovementSpace();
        void SelectAll();
        void SelectAllWithin(D3DXVECTOR2 topLeft, D3DXVECTOR2 bottomRight);
        void Load(const nlohmann::json &root);
        void SetDirty(bool value);
        void ResetViews();
        std::string GetStats();
        bool ToggleFillMode();
        void CheckChanges();
        void SetTitle(std::string title, bool store = true);
        void UpdateViewMatrix();
        void WrapCursor();
        bool IsActorSelected(const boost::uuids::uuid &id);
        void RefreshGizmo();
        void DragGizmo(const D3DXVECTOR2 &mousePos, boost::uuids::uuid &groupId);
        void DragToSelect(const D3DXVECTOR2 &mousePos, std::tuple<D3DXVECTOR2, ImVec2> &selectStart, std::tuple<D3DXVECTOR2, ImVec2> &selectStop);

    private:
        int m_version;
        HWND m_hWnd;
        D3DMATERIAL9 m_defaultMaterial;
        D3DFILLMODE m_fillMode;
        Gizmo m_gizmo;
        std::array<View, 5> m_views;
        std::map<boost::uuids::uuid, std::unique_ptr<Actor>> m_actors;
        Grid m_grid;
        std::vector<boost::uuids::uuid> m_selectedActorIds;
        float m_mouseSmoothX, m_mouseSmoothY;
        ViewType m_activeViewType;
        std::string m_sceneName;
        std::array<int, 3> m_backgroundColorRGB;
        Auditor m_auditor;
        Gui *m_gui;
        RenderDevice m_renderDevice;
        std::filesystem::path m_path;
        bool m_isDragging;
        bool m_isSelecting;
    };
}

#endif
