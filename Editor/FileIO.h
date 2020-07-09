#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <cJSON/cJSON.h>
#include <MicroTar/microtar.h>
#include "Scene.h"

#define LINE_FORMAT_LENGTH 128

using namespace std;

namespace UltraEd
{
    enum class FileType
    {
        Unknown, User, Editor
    };

    typedef struct
    {
        string path;
        FileType type;
    } FileInfo;

    class FileIO
    {
    public:
        static bool Save(Scene *scene, string &fileName);
        static bool Load(cJSON **data, string &fileName);
        static FileInfo Import(const char *file);
        static bool Pack(const char *path);
        static bool Unpack(const char *path);

    private:
        FileIO() {}
        static bool Compress(const string &path);
        static bool Decompress(string &path);
        static string CleanFileName(const char *fileName);
        static void TarifyFile(mtar_t *tar, const char *file);
        static void CreateDirectoryRecursively(const char *path);
    };
}

#endif
