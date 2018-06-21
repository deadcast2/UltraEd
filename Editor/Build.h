#ifndef _BUILD_H_
#define _BUILD_H_

#pragma warning(disable: 4786)

#include <windows.h>
#include <string>
#include <vector>
#include "gameobject.h"
#include "settings.h"
#include "shlwapi.h"

using namespace std;

class CBuild
{
public:
  static bool Start(vector<CGameObject*> gameObjects);
  static bool Run();
  static bool Load();

private:
  static bool Compile();
  static bool SaveMeshDataToFile(string fileName, CGameObject *gameObject);
  static bool AppendSegment(string objectId, string modelName, string *specSegments, string *specIncludes, string *romSegments);
};

#endif