#ifndef _RECORDS_H_
#define _RECORDS_H_

#include <rpc.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace UltraEd
{
    enum class AssetType
    {
        Unknown, Model, Texture
    };

    class AssetRecord
    {
    public:
        GUID id;
        AssetType type;
        GUID purgeId;
        std::string sourcePath;
        long long lastModified;
    };

    class ProjectRecord
    {
    public:
        std::string name;
        int version;
        std::vector<AssetRecord> assets;
    };
}

#endif
