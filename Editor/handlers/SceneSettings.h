#pragma once

#include <commdlg.h>
#include <stdio.h>
#include <string>
#include "../resource.h"

using namespace std;

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
                break;
            }
            case WM_CTLCOLORSTATIC:
            {
                if ((HWND)lParam == GetDlgItem(hWndDlg, IDC_EDIT_SCENE_COLOR_PREVIEW)
                    && settingsBrush)
                {
                    return (INT_PTR)settingsBrush;
                }
                break;
            }
            case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                    case IDC_EDIT_SCENE_COLOR_RED:
                    case IDC_EDIT_SCENE_COLOR_GREEN:
                    case IDC_EDIT_SCENE_COLOR_BLUE:
                    {
                        // Remove non-numerical characters and limit color value < 256.
                        if (HIWORD(wParam) == EN_CHANGE)
                        {
                            char buffer[4];
                            GetDlgItemText(hWndDlg, LOWORD(wParam), buffer, 4);

                            // Build new string of only numbers.
                            string cleanedValue;
                            for (int i = 0; i < strlen(buffer); i++)
                            {
                                if (isdigit(buffer[i]))
                                    cleanedValue.append(1, buffer[i]);
                            }

                            if (cleanedValue.size() > 0 && stoi(cleanedValue) > 255)
                                cleanedValue = "255";

                            // Stop sending update message when strings finally match.
                            if (strncmp(cleanedValue.c_str(), buffer, 3) != 0)
                                SetDlgItemText(hWndDlg, LOWORD(wParam), cleanedValue.c_str());
                        }
                        break;
                    }
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
