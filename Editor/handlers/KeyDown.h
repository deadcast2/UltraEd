#pragma once

#include <commctrl.h>

namespace UltraEd
{
    void KeyDownHandler(WORD key, CScene *scene)
    {
        if (scene == NULL) return;

        switch (key)
        {
            case VK_DELETE:
                scene->Delete();
                break;
            case 'D':
                if (GetKeyState(VK_CONTROL) & 0x8000) scene->Duplicate();
                break;
            case 'F':
                scene->FocusSelected();
                break;
            case 0x31: // 1 Key
                scene->SetGizmoModifier(UltraEd::GizmoModifierState::Translate);
                break;
            case 0x32: // 2 Key
                scene->SetGizmoModifier(UltraEd::GizmoModifierState::Rotate);
                break;
            case 0x33: // 3 Key
                scene->SetGizmoModifier(UltraEd::GizmoModifierState::Scale);
                break;
        }
    }
}
