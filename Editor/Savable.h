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
  virtual bool Load(char *data) = 0;

protected:
  std::map<char*, char*> dictionary;
};

#endif