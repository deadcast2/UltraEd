#pragma once

#include <commctrl.h>

namespace UltraEd
{
    HWND toolbarWindow;

    void ToolbarHandler(WPARAM wParam, CScene *scene)
    {
        if (scene == NULL) return;

        switch (LOWORD(wParam))
        {
            case IDM_TOOLBAR_TRANSLATE:
                scene->SetGizmoModifier(GizmoModifierState::Translate);
                break;
            case IDM_TOOLBAR_ROTATE:
                scene->SetGizmoModifier(GizmoModifierState::Rotate);
                break;
            case IDM_TOOLBAR_SCALE:
                scene->SetGizmoModifier(GizmoModifierState::Scale);
                break;
            case IDM_TOOLBAR_VIEW_PERSPECTIVE:
                scene->SetViewType(ViewType::Perspective);
                break;
            case IDM_TOOLBAR_VIEW_TOP:
                scene->SetViewType(ViewType::Top);
                break;
            case IDM_TOOLBAR_VIEW_FRONT:
                scene->SetViewType(ViewType::Front);
                break;
            case IDM_TOOLBAR_VIEW_LEFT:
                scene->SetViewType(ViewType::Left);
                break;
        }
    }

    HWND CreateToolbar(HWND hWnd, HINSTANCE hInst)
    {
        TBBUTTON tbrButtons[8];
        tbrButtons[0].idCommand = IDM_TOOLBAR_TRANSLATE;
        tbrButtons[0].fsState = TBSTATE_ENABLED;
        tbrButtons[0].fsStyle = TBSTYLE_BUTTON;
        tbrButtons[0].iBitmap = 0;
        tbrButtons[0].iString = (INT_PTR)L"Move";

        tbrButtons[1].idCommand = IDM_TOOLBAR_ROTATE;
        tbrButtons[1].fsState = TBSTATE_ENABLED;
        tbrButtons[1].fsStyle = TBSTYLE_BUTTON;
        tbrButtons[1].iBitmap = 1;
        tbrButtons[1].iString = (INT_PTR)L"Rotate";

        tbrButtons[2].idCommand = IDM_TOOLBAR_SCALE;
        tbrButtons[2].fsState = TBSTATE_ENABLED;
        tbrButtons[2].fsStyle = TBSTYLE_BUTTON;
        tbrButtons[2].iBitmap = 2;
        tbrButtons[2].iString = (INT_PTR)L"Scale";

        tbrButtons[3].fsState = TBSTATE_ENABLED;
        tbrButtons[3].fsStyle = TBSTYLE_SEP;
        tbrButtons[3].iBitmap = 0;

        tbrButtons[4].idCommand = IDM_TOOLBAR_VIEW_PERSPECTIVE;
        tbrButtons[4].fsState = TBSTATE_ENABLED;
        tbrButtons[4].fsStyle = TBSTYLE_BUTTON;
        tbrButtons[4].iBitmap = 3;
        tbrButtons[4].iString = (INT_PTR)L"Persp.";

        tbrButtons[5].idCommand = IDM_TOOLBAR_VIEW_TOP;
        tbrButtons[5].fsState = TBSTATE_ENABLED;
        tbrButtons[5].fsStyle = TBSTYLE_BUTTON;
        tbrButtons[5].iBitmap = 4;
        tbrButtons[5].iString = (INT_PTR)L"Top";

        tbrButtons[6].idCommand = IDM_TOOLBAR_VIEW_LEFT;
        tbrButtons[6].fsState = TBSTATE_ENABLED;
        tbrButtons[6].fsStyle = TBSTYLE_BUTTON;
        tbrButtons[6].iBitmap = 5;
        tbrButtons[6].iString = (INT_PTR)L"Left";

        tbrButtons[7].idCommand = IDM_TOOLBAR_VIEW_FRONT;
        tbrButtons[7].fsState = TBSTATE_ENABLED;
        tbrButtons[7].fsStyle = TBSTYLE_BUTTON;
        tbrButtons[7].iBitmap = 6;
        tbrButtons[7].iString = (INT_PTR)L"Front";

        return CreateToolbarEx(hWnd,
            WS_VISIBLE | WS_CHILD | WS_BORDER | TBSTYLE_TOOLTIPS,
            IDB_TOOLBAR,
            8,
            hInst,
            IDB_TOOLBAR,
            tbrButtons,
            8,
            16, 16, 16, 16,
            sizeof(TBBUTTON));
    }
}
