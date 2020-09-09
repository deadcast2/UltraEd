#ifndef _RECORDS_H_
#define _RECORDS_H_

#include <boost/uuid/uuid.hpp>
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
        boost::uuids::uuid id;
        AssetType type;
        boost::uuids::uuid purgeId;
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
