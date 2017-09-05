#ifndef _FILE_IO_H_
#define _FILE_IO_H_

#define LINE_FORMAT_LENGTH 128

#include "Scene.h"

enum FileType { Unknown, User, Editor };

typedef struct
{
  std::string path;
  FileType type;
} FileInfo;

class CFileIO
{
public:
  static bool Save(std::vector<CSavable*> savables);
  static bool Load(cJSON **data);
  static FileInfo Import(const char *file);

private:
  CFileIO() {}
  static bool Compress(const char *path);
  static bool Decompress(char **path);
  static std::string RootPath();
};

#endif