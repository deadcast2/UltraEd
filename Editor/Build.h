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
  static bool WriteSpecFile(vector<CGameObject*> gameObjects);
  static bool WriteSegmentsFile(vector<CGameObject*> gameObjects);
  static bool WriteModelsFile(vector<CGameObject*> gameObjects);
  static bool WriteCamerasFile(vector<CGameObject*> gameObjects);
  static bool WriteScriptsFile(vector<CGameObject*> gameObjects);
  static bool WriteMappingsFile(vector<CGameObject*> gameObjects);
  static bool WriteCollisionsFile(vector<CGameObject*> gameObjects);

private:
  static bool Compile();
  static bool SaveMeshDataToFile(string fileName, CGameObject *gameObject);
  static bool AppendSegment(string objectId, string modelName, string *specSegments, string *specIncludes, string *romSegments);
};

#endif