#include "Settings.h"
#include "Registry.h"

namespace UltraEd 
{
    void Settings::SetBuildCart(BuildCart cart)
    {
        Registry::Set("BuildCart", std::to_string(static_cast<int>(cart)));
    }

    BuildCart Settings::GetBuildCart()
    {
        std::string cart;

        if (Registry::Get("BuildCart", cart))
        {
            return static_cast<BuildCart>(atoi(cart.c_str()));
        }

        return BuildCart::_64drive;
    }

    void Settings::SetVideoMode(VideoMode mode)
    {
        Registry::Set("VideoMode", std::to_string(static_cast<int>(mode)));
    }

    VideoMode Settings::GetVideoMode()
    {
        std::string mode;

        if (Registry::Get("VideoMode", mode))
        {
            return static_cast<VideoMode>(atoi(mode.c_str()));
        }

        return VideoMode::NTSC;
    }

    void Settings::SetColorTheme(ColorTheme theme)
    {
        Registry::Set("ColorTheme", std::to_string(static_cast<int>(theme)));
    }

    ColorTheme Settings::GetColorTheme()
    {
        std::string theme;

        if (Registry::Get("ColorTheme", theme))
        {
            return static_cast<ColorTheme>(atoi(theme.c_str()));
        }

        return ColorTheme::Light;
    }

    void Settings::SetSaveUponBuild(bool save)
    {
        Registry::Set("SaveUponBuild", std::to_string(save ? 1 : 0));
    }

    bool Settings::GetSaveUponBuild()
    {
        std::string save;

        if (Registry::Get("SaveUponBuild", save))
        {
            return atoi(save.c_str()) == 1;
        }

        return true;
    }
}
