#ifndef _BUILD_H_
#define _BUILD_H_

#include <windows.h>
#include <string>
#include <vector>
#include "actor.h"
#include "Scene.h"

using namespace std;

namespace UltraEd
{
    class Build
    {
    public:
        static bool Start(Scene *scene);
        static bool Run();
        static bool Load(const HWND &hWnd);
        static bool WriteSpecFile(const vector<Actor*> &actors);
        static bool WriteDefinitionsFile();
        static bool WriteSegmentsFile(const vector<Actor*> &actors, map<string, string> *resourceCache);
        static bool WriteSceneFile(Scene *scene);
        static bool WriteActorsFile(const vector<Actor*> &actors, const map<string, string> &resourceCache);
        static bool WriteCollisionFile(const vector<Actor*> &actors);
        static bool WriteScriptsFile(const vector<Actor*> &actors);
        static bool WriteMappingsFile(const vector<Actor*> &actors);

    private:
        static bool Compile(const HWND &hWnd);
        static string GetPathFor(const string &name);
    };
}

#endif
