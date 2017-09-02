#ifndef _FILE_IO_H_
#define _FILE_IO_H_

#include "Scene.h"

enum FileType { Unknown, User, Editor };

typedef struct
{
  char* path;
  FileType type;
} FileInfo;

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
  FileInfo Import(const char* file);

private:
  bool Compress(const char* path);
  bool Decompress(char** path);

private:
  char startingDir[128];
};

#endif