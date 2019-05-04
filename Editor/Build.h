#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "gameobject.h"
#include "settings.h"
#include "shlwapi.h"

using namespace std;

namespace UltraEd
{
	class CBuild
	{
	public:
		static bool Start(vector<CActor*> gameObjects);
		static bool Run();
		static bool Load();
		static bool WriteSpecFile(vector<CActor*> gameObjects);
		static bool WriteSegmentsFile(vector<CActor*> gameObjects);
		static bool WriteModelsFile(vector<CActor*> gameObjects);
		static bool WriteCamerasFile(vector<CActor*> gameObjects);
		static bool WriteScriptsFile(vector<CActor*> gameObjects);
		static bool WriteMappingsFile(vector<CActor*> gameObjects);
		static bool WriteCollisionsFile(vector<CActor*> gameObjects);

	private:
		static bool Compile();
	};
}
