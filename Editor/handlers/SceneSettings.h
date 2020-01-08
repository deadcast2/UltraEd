#pragma once

#include <commdlg.h>
#include <stdio.h>
#include "../resource.h"

namespace UltraEd
{
    BOOL CALLBACK SceneSettingsProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            case WM_INITDIALOG:
            {
            }
            break;
            case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                    case IDC_BUTTON_SCENE_COLOR_CHOOSE:
                    {
                        DWORD rgbCurrent = 0;
                        COLORREF acrCustClr[16];
                        CHOOSECOLOR cc;
                        ZeroMemory(&cc, sizeof(cc));
                        cc.lStructSize = sizeof(cc);
                        cc.hwndOwner = hWndDlg;
                        cc.lpCustColors = (LPDWORD)acrCustClr;
                        cc.rgbResult = rgbCurrent;
                        cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                        if (ChooseColor(&cc) == TRUE)
                        {
                            rgbCurrent = cc.rgbResult;
                            char buffer[4];

                            sprintf(buffer, "%i", GetRValue(rgbCurrent));
                            SetDlgItemText(hWndDlg, IDC_EDIT_SCENE_COLOR_RED, buffer);

                            sprintf(buffer, "%i", GetGValue(rgbCurrent));
                            SetDlgItemText(hWndDlg, IDC_EDIT_SCENE_COLOR_GREEN, buffer);

                            sprintf(buffer, "%i", GetBValue(rgbCurrent));
                            SetDlgItemText(hWndDlg, IDC_EDIT_SCENE_COLOR_BLUE, buffer);
                        }
                        break;
                    }
                    case IDOK:
                    case IDCANCEL:
                        EndDialog(hWndDlg, wParam);
                        return TRUE;
                }
        }
        return FALSE;
    }
}
