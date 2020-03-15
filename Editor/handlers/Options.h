#pragma once

#include <commdlg.h>
#include <stdio.h>
#include <string>
#include "../resource.h"
#include "../Settings.h"

using namespace std;

namespace UltraEd
{
    BOOL CALLBACK OptionsProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            case WM_INITDIALOG:
            {
                HWND videoMode = GetDlgItem(hWndDlg, IDC_VIDEO_MODE);
                if (videoMode != NULL)
                {
                    SendMessage(videoMode, CB_ADDSTRING, 0, (LPARAM)_T("NTSC"));
                    SendMessage(videoMode, CB_ADDSTRING, 0, (LPARAM)_T("PAL"));
                    SendMessage(videoMode, CB_SETCURSEL, static_cast<int>(CSettings::GetVideoMode()), 0);
                }

                HWND buildCart = GetDlgItem(hWndDlg, IDC_BUILD_CART);
                if (buildCart != NULL)
                {
                    SendMessage(buildCart, CB_ADDSTRING, 0, (LPARAM)_T("64drive"));
                    SendMessage(buildCart, CB_ADDSTRING, 0, (LPARAM)_T("EverDrive-64 X7"));
                    SendMessage(buildCart, CB_SETCURSEL, static_cast<int>(CSettings::GetBuildCart()), 0);
                }
                break;
            }
            case WM_COMMAND:
            {
                switch (LOWORD(wParam))
                {
                    case IDOK:
                    {
                        HWND videoMode = GetDlgItem(hWndDlg, IDC_VIDEO_MODE);
                        if (videoMode != NULL)
                        {
                            int index = SendMessage(videoMode, CB_GETCURSEL, 0, 0);
                            CSettings::SetVideoMode(static_cast<VideoMode>(index));
                        }

                        HWND buildCart = GetDlgItem(hWndDlg, IDC_BUILD_CART);
                        if (buildCart != NULL)
                        {
                            int index = SendMessage(buildCart, CB_GETCURSEL, 0, 0);
                            CSettings::SetBuildCart(static_cast<BuildCart>(index));
                        }
                    }
                    case IDCANCEL:
                    {
                        EndDialog(hWndDlg, wParam);
                        return TRUE;
                    }
                }
            }
        }
        return FALSE;
    }
}
