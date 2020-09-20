#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <nlohmann/json.hpp>
#include <MicroTar/microtar.h>
#include "Scene.h"

#define LINE_FORMAT_LENGTH 128

namespace UltraEd
{
    class FileIO
    {
    public:
        static bool Save(Scene *scene, const std::filesystem::path &path);
        static bool Load(std::shared_ptr<nlohmann::json> &data, const std::filesystem::path &path);
        static bool Pack(const char *path);
        static bool Unpack(const char *path);

    private:
        FileIO() {}
        static bool Compress(const std::string &path);
        static bool Decompress(std::string &path);
        static void TarifyFile(mtar_t *tar, const char *file);
        static void CreateDirectoryRecursively(const char *path);
    };
}

#endif
