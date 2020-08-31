#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <d3d9.h>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <rpc.h>
#include "Common.h"
#include "ModelPreviewer.h"

using namespace std::filesystem;
using json = nlohmann::json;

namespace UltraEd
{
    enum class AssetType { Unknown, Model, Texture };

    class Project
    {
    public:
        Project();
        Project(const path &path);
        Project(const char *name, const path &path, bool createDirectory);
        ~Project();
        bool Save();
        const std::map<GUID, LPDIRECT3DTEXTURE9> Textures(LPDIRECT3DDEVICE9 device);
        const std::map<GUID, LPDIRECT3DTEXTURE9> Models(LPDIRECT3DDEVICE9 device);

    private:
        path ParentPath();
        path LibraryPath(const json &asset = 0);
        AssetType DetectAssetType(const path &path);
        json GetAsset(GUID id);
        void Scan();
        void Load(const json &database);
        void PreparePreview(const AssetType &type, const GUID &id);
        void RemovePreview(const AssetType &type, const GUID &id);
        void AddAsset(const AssetType &type, const directory_entry &entry);
        void UpdateAsset(const AssetType &type, const directory_entry &entry);
        bool IsValidDatabase(const json &database);
        bool IsValidFile(const directory_entry &entry);
        bool IsSupportedModel(const path &path);
        bool IsSupportedTexture(const path &path);
        bool IsAssetModified(const AssetType &type, const directory_entry &entry);
        bool AssetExists(const AssetType &type, const path &path);
        bool InsertAsset(const AssetType &type, const path &path);
        bool InitializeLibrary();
        void VerifyAsset(const GUID &purgeId, const AssetType &type, const directory_entry &entry);
        void PurgeMissingAssets(const GUID &purgeId);

    private:
        std::string m_name;
        path m_databasePath;
        std::map<AssetType, std::map<path, json>> m_assetIndex;
        std::map<AssetType, std::string> m_assetTypeNames;
        std::set<std::string> m_modelExtensions;
        std::set<std::string> m_textureExtensions;
        std::function<void()> m_activateSubscriber;
        std::map<AssetType, std::map<GUID, LPDIRECT3DTEXTURE9>> m_assetPreviews;
        ModelPreviewer m_modelPreviewer;
    };
}

#endif
