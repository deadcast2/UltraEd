#pragma once

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
                    case IDOK:
                    {
                    }
                    case IDCANCEL:
                        EndDialog(hWndDlg, wParam);
                        return TRUE;
                }
        }
        return FALSE;
    }
}
