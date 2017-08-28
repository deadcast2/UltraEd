#ifndef _FILE_IO_H_
#define _FILE_IO_H_

#include "Scene.h"

class CFileIO
{
public:
  static CFileIO& Instance()
  {
    static CFileIO instance;
    return instance;
  }
  bool Save(std::vector<CSavable*> savables);
  bool Load(char** data);
  char* Copy(const char* file, bool makeUnique);

private:
  char startingDir[128];
};

#endif