#ifndef _FILE_IO_H_
#define _FILE_IO_H_

#include "Scene.h"

class CFileIO
{
public:
  CFileIO();
  ~CFileIO();
  BOOL Save(std::vector<CSavable*> savables);
};

#endif