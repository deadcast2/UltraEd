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
#include "Debug.h"
#include "Gizmo.h"
#include "Grid.h"
#include "Model.h"
#include "Camera.h"
#include "Auditor.h"

using namespace std;

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
        vector<Actor *> GetActors();
        COLORREF GetBackgroundColor();
        HWND GetWndHandle();
        void Render();
        cJSON *Save();
        cJSON *PartialSave(cJSON *root);
        bool PartialLoad(cJSON *root);
        void UnselectAll();
        shared_ptr<Actor> GetActor(GUID id);
        void RestoreActor(cJSON *item);
        void Delete(shared_ptr<Actor> actor);
        void SelectActorById(GUID id, bool clearAll = true);
     
    private:
        void Delete();
        void Duplicate();
        void FocusSelected();
        void SetScript(string script);
        string GetScript();
        void SetBackgroundColor(COLORREF color);
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
        void Undo();
        void Redo();
        bool Pick(POINT mousePoint, Actor **selectedActor = NULL);
        void ReleaseResources(ModelRelease type);
        void ScreenRaycast(POINT screenPoint, D3DXVECTOR3 *origin, D3DXVECTOR3 *dir);
        void SetViewType(ViewType type);
        void SetGizmoModifier(GizmoModifierState state);
        void SetGizmoSnapSize(float size);
        float GetGizmoSnapSize();
        View *GetActiveView();
        bool ToggleMovementSpace();
        bool ToggleSnapToGrid();
        void SelectAll();
        bool Load(cJSON *root);
        void SetDirty(bool value);
        void ResetViews();
        string GetStats();
        bool ToggleFillMode();
        void CheckChanges();
        void CheckInput(const float deltaTime);
        void SetTitle(string title, bool store = true);
        void UpdateViewMatrix();
        void RefreshActorList();
        void WrapCursor();

    private:
        D3DLIGHT9 m_worldLight;
        D3DMATERIAL9 m_defaultMaterial;
        D3DMATERIAL9 m_selectedMaterial;
        D3DFILLMODE m_fillMode;
        Gizmo m_gizmo;
        View m_views[4];
        IDirect3DDevice9 *m_device;
        IDirect3D9 *m_d3d9;
        D3DPRESENT_PARAMETERS m_d3dpp;
        map<GUID, shared_ptr<Actor>> m_actors;
        Grid m_grid;
        vector<GUID> m_selectedActorIds;
        float m_mouseSmoothX, m_mouseSmoothY;
        ViewType m_activeViewType;
        string m_sceneName;
        array<int, 3> m_backgroundColorRGB;
        Auditor m_auditor;
        unique_ptr<Gui> m_gui;
    };
}

#endif
