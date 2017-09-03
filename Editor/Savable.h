#ifndef _SAVABLE_H_
#define _SAVABLE_H_

#pragma warning(disable: 4786)

#include <map>
#include "cJSON.h"

struct SavableType
{
  enum Value { Editor, Model };
};

typedef struct
{
  cJSON* object;
  SavableType::Value type;
} Savable;

class CSavable
{
public:
  virtual Savable Save() = 0;
  virtual bool Load(cJSON* root) = 0;
  std::map<char*, char*> GetResources()
  {
    return resources;
  };

protected:
  std::map<char*, char*> resources;
};

#endif