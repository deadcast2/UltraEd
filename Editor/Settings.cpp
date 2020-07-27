#include "Settings.h"
#include "Registry.h"

namespace UltraEd 
{
    void Settings::SetBuildCart(BuildCart cart)
    {
        Registry::Set("BuildCart", to_string(static_cast<int>(cart)));
    }

    BuildCart Settings::GetBuildCart()
    {
        string cart;
        if (Registry::Get("BuildCart", cart))
        {
            return static_cast<BuildCart>(atoi(cart.c_str()));
        }
        return BuildCart::_64drive;
    }

    void Settings::SetVideoMode(VideoMode mode)
    {
        Registry::Set("VideoMode", to_string(static_cast<int>(mode)));
    }

    VideoMode Settings::GetVideoMode()
    {
        string mode;
        if (Registry::Get("VideoMode", mode))
        {
            return static_cast<VideoMode>(atoi(mode.c_str()));
        }
        return VideoMode::NTSC;
    }

    void Settings::SetColorTheme(ColorTheme theme)
    {
        Registry::Set("ColorTheme", to_string(static_cast<int>(theme)));
    }

    ColorTheme Settings::GetColorTheme()
    {
        string theme;
        if (Registry::Get("ColorTheme", theme))
        {
            return static_cast<ColorTheme>(atoi(theme.c_str()));
        }
        return ColorTheme::Light;
    }
}
