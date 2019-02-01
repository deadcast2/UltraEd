#ifndef _FILE_IO_H_
#define _FILE_IO_H_

#define LINE_FORMAT_LENGTH 128

#include "Scene.h"
#include "microtar.h"

using namespace std;

enum FileType { Unknown, User, Editor };

typedef struct
{
  string path;
  FileType type;
} FileInfo;

class CFileIO
{
public:
  static bool Save(vector<CSavable*> savables, string &fileName);
  static bool Load(cJSON **data, string &fileName);
  static FileInfo Import(const char *file);
  static bool Pack(const char *path);
  static bool Unpack(const char *path);

private:
  CFileIO() {}
  static bool Compress(string path);
  static bool Decompress(string &path);
  static string CleanFileName(const char *fileName);
  static void TarifyFile(mtar_t *tar, const char *file);
};

#endif