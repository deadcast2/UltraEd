#pragma once

#include <windows.h>
#include <string>

#define REG_DATA_LENGTH SIZE_MAX

using namespace std;

namespace UltraEd
{
    enum class BuildCart { _64drive, EverDrive };

    enum class VideoMode { NTSC, PAL };

    class CSettings
    {
    public:
        static void SetBuildCart(BuildCart cart);
        static BuildCart GetBuildCart();
        static void SetVideoMode(VideoMode mode);
        static VideoMode GetVideoMode();
    };
}
