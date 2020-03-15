#include "Settings.h"
#include "Registry.h"

namespace UltraEd 
{
    void CSettings::SetVideoMode(VideoMode mode)
    {
        CRegistry::Set("VideoMode", to_string(static_cast<int>(mode)));
    }

    VideoMode CSettings::GetVideoMode()
    {
        string mode;
        if (CRegistry::Get("VideoMode", mode))
        {
            return static_cast<VideoMode>(atoi(mode.c_str()));
        }
        return VideoMode::NTSC;
    }
}
