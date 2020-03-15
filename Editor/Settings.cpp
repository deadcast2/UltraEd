#include "Settings.h"
#include "Registry.h"

namespace UltraEd 
{
    void CSettings::SetBuildCart(BuildCart cart)
    {
        CRegistry::Set("BuildCart", to_string(static_cast<int>(cart)));
    }

    BuildCart CSettings::GetBuildCart()
    {
        string cart;
        if (CRegistry::Get("BuildCart", cart))
        {
            return static_cast<BuildCart>(atoi(cart.c_str()));
        }
        return BuildCart::_64drive;
    }

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
