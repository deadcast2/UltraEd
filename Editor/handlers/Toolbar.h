#pragma once

#include <windows.h>
#include "../Scene.h"

#define IDM_TOOLBAR_TRANSLATE 5000
#define IDM_TOOLBAR_ROTATE 5001
#define IDM_TOOLBAR_SCALE 5002
#define IDM_TOOLBAR_VIEW_PERSPECTIVE 5003
#define IDM_TOOLBAR_VIEW_TOP 5004
#define IDM_TOOLBAR_VIEW_LEFT 5005
#define IDM_TOOLBAR_VIEW_FRONT 5006

namespace UltraEd
{
    void ToolbarHandler(WPARAM wParam, CScene &scene)
    {
        switch (LOWORD(wParam))
        {
            case IDM_TOOLBAR_TRANSLATE:
                scene.SetGizmoModifier(Translate);
                break;
            case IDM_TOOLBAR_ROTATE:
                scene.SetGizmoModifier(Rotate);
                break;
            case IDM_TOOLBAR_SCALE:
                scene.SetGizmoModifier(Scale);
                break;
            case IDM_TOOLBAR_VIEW_PERSPECTIVE:
                scene.SetViewType(ViewType::Perspective);
                break;
            case IDM_TOOLBAR_VIEW_TOP:
                scene.SetViewType(ViewType::Top);
                break;
            case IDM_TOOLBAR_VIEW_FRONT:
                scene.SetViewType(ViewType::Front);
                break;
            case IDM_TOOLBAR_VIEW_LEFT:
                scene.SetViewType(ViewType::Left);
                break;
        }
    }
}
