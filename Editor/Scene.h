#ifndef _SCENE_H_
#define _SCENE_H_

#include <array>
#include <map>
#include <d3d9.h>
#include <d3dx9.h>
#include <cJSON/cJSON.h>
#include "Gui.h"
#include "View.h"
#include "Common.h"
#include "Gizmo.h"
#include "Grid.h"
#include "Model.h"
#include "Camera.h"
#include "Auditor.h"

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
        Scene();
        ~Scene();
        bool Create(HWND hWnd);
        bool Confirm();
        std::vector<Actor *> GetActors(bool selectedOnly = false);
        COLORREF GetBackgroundColor();
        HWND GetWndHandle();
        void Render();
        cJSON *Save();
        cJSON *PartialSave(cJSON *root);
        bool PartialLoad(cJSON *root);
        void UnselectAll();
        std::shared_ptr<Actor> GetActor(GUID id);
        void RestoreActor(cJSON *item);
        void Delete(std::shared_ptr<Actor> actor);
        void SelectActorById(GUID id, bool clearAll = true);
     
    private:
        void Delete();
        void Duplicate();
        void FocusSelected();
        void SetScript(std::string script);
        std::string GetScript();
        void SetBackgroundColor(COLORREF color);
        void SetGizmoSnapSize(float size);
        void Resize(int width, int height);
        void OnNew(bool confirm = true);
        bool OnSave();
        void OnLoad();
        void OnAddCamera();
        void OnAddTexture();
        void OnDeleteTexture();
        void OnAddModel(ModelPreset preset);
        void OnAddCollider(ColliderType type);
        void OnDeleteCollider();
        void OnBuildROM(BuildFlag flag);
        bool Pick(ImVec2 mousePoint, bool ignoreGizmo = false, Actor **selectedActor = NULL);
        void ReleaseResources(ModelRelease type);
        void ScreenRaycast(ImVec2 screenPoint, D3DXVECTOR3 *origin, D3DXVECTOR3 *dir);
        void SetViewType(ViewType type);
        View *GetActiveView();
        bool ToggleMovementSpace();
        void SelectAll();
        bool Load(cJSON *root);
        void SetDirty(bool value);
        void ResetViews();
        std::string GetStats();
        bool ToggleFillMode();
        void CheckChanges();
        void CheckInput();
        void SetTitle(std::string title, bool store = true);
        void UpdateViewMatrix();
        void WrapCursor();
        bool IsActorSelected(GUID id);

    private:
        D3DMATERIAL9 m_defaultMaterial;
        D3DFILLMODE m_fillMode;
        Gizmo m_gizmo;
        View m_views[4];
        IDirect3DDevice9 *m_device;
        IDirect3D9 *m_d3d9;
        D3DPRESENT_PARAMETERS m_d3dpp;
        std::map<GUID, std::shared_ptr<Actor>> m_actors;
        Grid m_grid;
        std::vector<GUID> m_selectedActorIds;
        float m_mouseSmoothX, m_mouseSmoothY;
        ViewType m_activeViewType;
        std::string m_sceneName;
        std::array<int, 3> m_backgroundColorRGB;
        Auditor m_auditor;
        std::unique_ptr<Gui> m_gui;
    };
}

#endif
