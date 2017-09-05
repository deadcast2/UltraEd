#ifndef _SAVABLE_H_
#define _SAVABLE_H_

#pragma warning(disable: 4786)

#include <map>
#include <d3d8.h>
#include "cJSON.h"

struct SavableType
{
  enum Value { Editor, Model };
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
  std::map<std::string, std::string> GetResources()
  {
    return resources;
  };

protected:
  std::map<std::string, std::string> resources;
};

#endif