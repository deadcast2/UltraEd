#pragma once

#include <commdlg.h>
#include <stdio.h>
#include <string>
#include "../resource.h"
#include "../Settings.h"

using namespace std;

namespace UltraEd
{
    enum class VideoMode { NTSC, PAL };

    VideoMode GetOptionsVideoMode()
    {
        string mode;
        if (CSettings::Get("VideoMode", mode))
        {
            return static_cast<VideoMode>(atoi(mode.c_str()));
        }
        return VideoMode::NTSC;
    }

    BOOL CALLBACK OptionsProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            case WM_INITDIALOG:
            {
                HWND comboBox = GetDlgItem(hWndDlg, IDC_VIDEO_MODE);
                SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)_T("NTSC"));
                SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)_T("PAL"));
                SendMessage(comboBox, CB_SETCURSEL, static_cast<int>(GetOptionsVideoMode()), 0);
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
                        CSettings::Set("VideoMode", to_string(index));
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
