#ifndef _FILE_IO_H_
#define _FILE_IO_H_

#include "Scene.h"

class CFileIO
{
public:
  CFileIO();
  ~CFileIO();
  bool Save(std::vector<CSavable*> savables);
  bool Load(char** data);
};

#endif