#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <map>
#include <nlohmann/json.hpp>
#include <set>

using namespace std::filesystem;
using json = nlohmann::json;

namespace UltraEd
{
    enum class AssetType { Unknown, Model, Texture };

    class Project
    {
    public:
        Project(const char *name, const path &path, bool createDirectory);
        void Save();

    private:
        path ParentPath();
        path LibraryPath();
        void Scan();
        AssetType DetectAssetType(const path &path);
        void AddAsset(const AssetType &type, const directory_entry &entry);
        void UpdateAsset(const AssetType &type, const directory_entry &entry);
        bool IsValidFile(const directory_entry &entry);
        bool IsSupportedModel(const path &path);
        bool IsSupportedTexture(const path &path);
        bool IsAssetModified(const AssetType &type, const directory_entry &entry);
        bool AssetExists(const AssetType &type, const path &path);
        bool InsertAsset(const AssetType &type, const path &path);
        bool InitializeLibrary();

    private:
        const char *m_name;
        path m_databasePath;
        std::map<AssetType, std::map<path, json>> m_assetIndex;
        std::map<AssetType, std::string> m_assetTypeNames;
        std::set<std::string> m_modelExtensions;
        std::set<std::string> m_textureExtensions;
    };
}

#endif
