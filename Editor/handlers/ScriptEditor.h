#pragma once

#include "../vendor/Scintilla.h"
#include "../vendor/SciLexer.h"

namespace UltraEd
{
    HWND scriptEditorWindow;

    INT_PTR CALLBACK ScriptEditorHandler(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {      
        switch (message)
        {
            case WM_INITDIALOG:
            {
                RECT rc;
                CScene *scene = (CScene*)lParam;
                GetClientRect(hWndDlg, &rc);
                scriptEditorWindow = CreateWindow(
                    "Scintilla",
                    "Source",
                    WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
                    0, 0,
                    rc.right, rc.bottom - 50,
                    hWndDlg,
                    0,
                    0,
                    0);
                SendMessage(scriptEditorWindow, SCI_SETLEXER, SCLEX_CPP, 0);
                SendMessage(scriptEditorWindow, SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
                SendMessage(scriptEditorWindow, SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<LPARAM>("Verdana"));

                if (scene != NULL)
                {
                    SendMessage(scriptEditorWindow, SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(scene->GetScript().c_str()));
                }

                ShowWindow(scriptEditorWindow, SW_SHOW);
                SetFocus(scriptEditorWindow);
                SetWindowLongPtr(scriptEditorWindow, GWLP_USERDATA, (LPARAM)scene);
                break;
            }
            case WM_COMMAND:
            {
                switch (LOWORD(wParam))
                {
                    case IDC_SCRIPT_EDITOR_SAVE_CHANGES:
                    {
                        CScene *scene = (CScene*)GetWindowLongPtr(scriptEditorWindow, GWLP_USERDATA);
                        HRESULT length = SendMessage(scriptEditorWindow, SCI_GETLENGTH, 0, 0) + 1;
                        char *buffer = (char*)malloc(sizeof(char) * length);
                        SendMessage(scriptEditorWindow, SCI_GETTEXT, length, reinterpret_cast<LPARAM>(buffer));
                        
                        if (scene != NULL) scene->SetScript(string(buffer));

                        free(buffer);
                        SendMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0);
                        return TRUE;
                    }
                    case IDCANCEL:
                        EndDialog(hWndDlg, wParam);
                        return TRUE;
                }
                break;
            }
        }
        return FALSE;
    }
}
