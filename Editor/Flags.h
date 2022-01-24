#ifndef _FLAGS_H_
#define _FLAGS_H_

namespace UltraEd
{
    enum class ActorType { Model, Camera };

    enum class AssetType { Unknown, Model, Texture };

    enum class BuildFlag { Unknown = 1, Build = 2, Run = 4, Load = 8 };

    enum class ColliderType { Box, Sphere };

    enum class GizmoAxisState { XAxis, YAxis, ZAxis };

    enum class GizmoModifierState { Translate, Scale, Rotate };

    enum class BuildCart { _64drive, EverDrive };

    enum class VideoMode { NTSC, PAL };
    
    enum class ColorTheme { Dark, Light };

    enum class ViewType { Perspective, Top, Left, Right, Front };

    enum class ModelPreset { Pumpkin };

    inline bool operator&(BuildFlag a, BuildFlag b)
    {
        return static_cast<int>(a) & static_cast<int>(b);
    }
}

#endif
