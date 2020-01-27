#pragma once

#include <windows.h>
#include "resource.h"

namespace UltraEd
{
    class IconLoader
    {
    public:
        ~IconLoader()
        {
            for (const HBITMAP &bitmap : m_bitmaps)
                DeleteObject(bitmap);
        }

        void SetMenuIcon(HWND hWnd, UINT menuId, UINT bitmapId)
        {
            HBITMAP bitmap = static_cast<HBITMAP>(LoadImage(GetModuleHandle(0),
                MAKEINTRESOURCE(bitmapId), IMAGE_BITMAP, 16, 16, LR_MONOCHROME));
            if (bitmap == NULL) return;

            MENUITEMINFO info = { 0 };
            info.cbSize = sizeof(MENUITEMINFO);
            info.fMask = MIIM_BITMAP;
            info.hbmpItem = bitmap;

            HMENU menu = GetMenu(hWnd);
            if (menu) SetMenuItemInfo(menu, menuId, 0, &info);

            m_bitmaps.push_back(bitmap);
        }
    private:
        vector<HBITMAP> m_bitmaps;
    };
}
