#ifndef _BUILD_H_
#define _BUILD_H_

#include <windows.h>
#include <string>
#include <vector>
#include "actor.h"
#include "Scene.h"

namespace UltraEd
{
    class Build
    {
    public:
        static bool Start(Scene *scene);
        static bool Run();
        static bool Load();

    private:
        static bool WriteSpecFile(const std::vector<Actor*> &actors);
        static bool WriteDefinitionsFile();
        static bool WriteSegmentsFile(const std::vector<Actor*> &actors, std::map<std::filesystem::path, std::string> *resourceCache);
        static bool WriteSceneFile(Scene *scene);
        static bool WriteActorsFile(const std::vector<Actor*> &actors, const std::map<std::filesystem::path, std::string> &resourceCache);
        static bool WriteCollisionFile(const std::vector<Actor*> &actors);
        static bool WriteScriptsFile(const std::vector<Actor*> &actors);
        static bool Compile();
    };
}

#endif
