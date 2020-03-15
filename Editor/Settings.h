#pragma once

#include <windows.h>
#include <string>

#define REG_DATA_LENGTH SIZE_MAX

using namespace std;

namespace UltraEd
{
    enum class VideoMode { NTSC, PAL };

    class CSettings
    {
    public:
        static void SetVideoMode(VideoMode mode);
        static VideoMode GetVideoMode();
    };
}
