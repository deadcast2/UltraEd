#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "actor.h"
#include "settings.h"
#include "shlwapi.h"

using namespace std;

namespace UltraEd
{
    class CBuild
    {
    public:
        static bool Start(vector<CActor*> actors);
        static bool Run();
        static bool Load();
        static bool WriteSpecFile(vector<CActor*> actors);
        static bool WriteSegmentsFile(vector<CActor*> actors);
        static bool WriteModelsFile(vector<CActor*> actors);
        static bool WriteCamerasFile(vector<CActor*> actors);
        static bool WriteScriptsFile(vector<CActor*> actors);
        static bool WriteMappingsFile(vector<CActor*> actors);

    private:
        static bool Compile();
    };
}
