#ifndef _SAVABLE_H_
#define _SAVABLE_H_

#include <map>
#include "deps/DXSDK/include/d3d8.h"
#include "cJSON.h"

using namespace std;

struct SavableType
{
  enum Value { Camera, GameObject };
};

typedef struct
{
  cJSON *object;
  SavableType::Value type;
} Savable;

class CSavable
{
public:
  virtual Savable Save() = 0;
  virtual bool Load(IDirect3DDevice8 *device, cJSON *root) = 0;
  map<string, string> GetResources()
  {
    return resources;
  };

protected:
  map<string, string> resources;
};

#endif