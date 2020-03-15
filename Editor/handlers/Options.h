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
                HWND comboBox = GetDlgItem(hWndDlg, IDC_VIDEO_MODE);
                SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)_T("NTSC"));
                SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)_T("PAL"));
                SendMessage(comboBox, CB_SETCURSEL, static_cast<int>(CSettings::GetVideoMode()), 0);
                SetWindowPos(comboBox, NULL, 0, 0, 80, 80, SWP_NOMOVE);
                break;
            }
            case WM_COMMAND:
            {
                switch (LOWORD(wParam))
                {
                    case IDOK:
                    {
                        HWND comboBox = GetDlgItem(hWndDlg, IDC_VIDEO_MODE);
                        int index = SendMessage(comboBox, CB_GETCURSEL, 0, 0);
                        CSettings::SetVideoMode(static_cast<VideoMode>(index));
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
