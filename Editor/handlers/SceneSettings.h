#pragma once

#include <commdlg.h>
#include <stdio.h>
#include "../resource.h"

namespace UltraEd
{
    HBRUSH settingsBrush;

    BOOL CALLBACK SceneSettingsProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            case WM_INITDIALOG:
            {
                settingsBrush = CreateSolidBrush(RGB(255, 255, 255));

                // Limit RGB fields to 3 characters max.
                SendDlgItemMessage(hWndDlg, IDC_EDIT_SCENE_COLOR_RED, EM_SETLIMITTEXT, 3, 0);
                SendDlgItemMessage(hWndDlg, IDC_EDIT_SCENE_COLOR_GREEN, EM_SETLIMITTEXT, 3, 0);
                SendDlgItemMessage(hWndDlg, IDC_EDIT_SCENE_COLOR_BLUE, EM_SETLIMITTEXT, 3, 0);
            }
            break;
            case WM_CTLCOLORSTATIC:
            {
                if ((HWND)lParam == GetDlgItem(hWndDlg, IDC_EDIT_SCENE_COLOR_PREVIEW)
                    && settingsBrush)
                {
                    return (INT_PTR)settingsBrush;
                }
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
                            DeleteObject(settingsBrush);
                            settingsBrush = CreateSolidBrush(cc.rgbResult);
                            InvalidateRect(GetDlgItem(hWndDlg, IDC_EDIT_SCENE_COLOR_PREVIEW), 0, 0);

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
                        DeleteObject(settingsBrush);
                        EndDialog(hWndDlg, wParam);
                        return TRUE;
                }
        }
        return FALSE;
    }
}
