#ifndef _SAVABLE_H_
#define _SAVABLE_H_

#pragma warning(disable: 4786)

#include <map>

class CSavable
{
public:
  virtual char* Save() = 0;
  virtual bool Load(char *data) = 0;

protected:
  std::map<char*, char*> resourcePaths;
};

#endif