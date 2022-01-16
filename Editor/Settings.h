#ifndef _SETTING_H_
#define _SETTING_H_

#include <windows.h>
#include <string>
#include "Flags.h"

#define REG_DATA_LENGTH SIZE_MAX

namespace UltraEd
{
    class Settings
    {
    public:
        static void SetBuildCart(BuildCart cart);
        static BuildCart GetBuildCart();
        static void SetVideoMode(VideoMode mode);
        static VideoMode GetVideoMode();
        static void SetColorTheme(ColorTheme theme);
        static ColorTheme GetColorTheme();
        static void SetSaveUponBuild(bool save);
        static bool GetSaveUponBuild();
    };
}

#endif
