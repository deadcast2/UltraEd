#pragma once

namespace UltraEd
{
    HWND tabsWindow;
    HWND buildOutput;
    int tabsWindowHeight = 130; // Starting height
    const int tabsBorder = 2;
    string buildOutputBuffer;

    void TabHandler(WPARAM wParam, LPARAM lParam)
    {
        switch (wParam)
        {
            case TAB_BUILD_OUTPUT_CLEAR:
                buildOutputBuffer.clear();
                break;
            case TAB_BUILD_OUTPUT:
                unique_ptr<char> output(reinterpret_cast<char *>(lParam));
                if (output != NULL)
                {
                    buildOutputBuffer.append(output.get());
                    SendMessage(buildOutput, WM_SETTEXT, 0, (LPARAM)buildOutputBuffer.c_str());
                    SendMessage(buildOutput, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), NULL);
                }
                break;
        }
    }
}
