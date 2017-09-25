#ifndef _BUILD_H_
#define _BUILD_H_

#pragma warning(disable: 4786)

#include <windows.h>
#include <string>
#include <vector>
#include "model.h"
#include "settings.h"
#include "shlwapi.h"

using namespace std;

class CBuild
{
public:
  static bool Start(vector<CModel*> models);

private:
  static bool Compile();
};

#endif