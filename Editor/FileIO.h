#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <cJSON/cJSON.h>
#include <MicroTar/microtar.h>
#include "Scene.h"

#define LINE_FORMAT_LENGTH 128

namespace UltraEd
{
    enum class FileType
    {
        Unknown, User, Editor
    };

    typedef struct
    {
        std::string path;
        FileType type;
    } FileInfo;

    class FileIO
    {
    public:
        static bool Save(Scene *scene, std::string &fileName);
        static bool Load(cJSON **data, std::string &fileName);
        static FileInfo Import(const char *file);
        static bool Pack(const char *path);
        static bool Unpack(const char *path);

    private:
        FileIO() {}
        static bool Compress(const std::string &path);
        static bool Decompress(std::string &path);
        static std::string CleanFileName(const char *fileName);
        static void TarifyFile(mtar_t *tar, const char *file);
        static void CreateDirectoryRecursively(const char *path);
    };
}

#endif
