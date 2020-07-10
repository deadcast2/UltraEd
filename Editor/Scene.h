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
        _, Run, Load
    };

    class Scene : public Savable
    {
    public:
        Scene();
        ~Scene();
        bool Create(HWND hWnd);
        void Delete();
        void Duplicate();
        void FocusSelected();
        void SetScript(string script);
        string GetScript();
        void SetBackgroundColor(COLORREF color);
        COLORREF GetBackgroundColor();
        void Render();
        void Resize(int width, int height);
        void OnMouseWheel(short zDelta);
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
        bool ToggleFillMode();
        bool ToggleSnapToGrid();
        void SelectActorById(GUID id, bool clearAll = true);
        void SelectAll();
        void UnselectAll();
        cJSON *Save();
        cJSON *PartialSave(cJSON *root);
        bool Load(cJSON *root);
        bool PartialLoad(cJSON *root);
        void SetDirty(bool value);
        bool Confirm();
        HWND GetWndHandle();
        vector<Actor *> GetActors();
        shared_ptr<Actor> GetActor(GUID id);
        void Delete(shared_ptr<Actor> actor);
        void RestoreActor(cJSON *item);
        void ResetViews();
        string GetStats();

    private:
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
