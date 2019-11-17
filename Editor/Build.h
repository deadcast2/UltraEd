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
        static bool Start(const vector<CActor*> &actors, const HWND &hWnd);
        static bool Run();
        static bool Load();
        static bool WriteSpecFile(const vector<CActor*> &actors);
        static bool WriteSegmentsFile(const vector<CActor*> &actors, map<string, string> *resourceCache);
        static bool WriteActorsFile(const vector<CActor*> &actors, const map<string, string> &resourceCache);
        static bool WriteCollisionFile(const vector<CActor*> &actors);
        static bool WriteScriptsFile(const vector<CActor*> &actors);
        static bool WriteMappingsFile(const vector<CActor*> &actors);

    private:
        static bool Compile(const HWND &hWnd);
        static string GetPathFor(const string &name);
    };
}
