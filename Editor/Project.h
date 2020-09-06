#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <d3d9.h>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <rpc.h>
#include "Common.h"
#include "ModelPreviewer.h"
#include "Records.h"

using namespace std::filesystem;
using json = nlohmann::json;

namespace UltraEd
{
    class Project
    {
        /// <summary>
        /// Trick to make ctors pseudo private while using unique_ptr. Method found here:
        /// https://seanmiddleditch.com/enabling-make-unique-with-private-constructors/
        /// </summary>
        struct m_constructor_tag
        {
            explicit m_constructor_tag() = default;
        };

    public:
        Project(m_constructor_tag tag);
        Project(m_constructor_tag tag, const path &path);
        Project(m_constructor_tag tag, const char *name, const path &path, bool createDirectory);
        ~Project();
        static void New(const char *name, const path &path, bool createDirectory);
        static void Load(const path &path);
        static void Activate();
        static bool Save(const char *name = 0);
        static bool IsLoaded();
        static std::map<GUID, LPDIRECT3DTEXTURE9> Previews(const AssetType &type, LPDIRECT3DDEVICE9 device);
    
    private:
        path ParentPath();
        path LibraryPath(const AssetRecord *record = 0);
        AssetType DetectAssetType(const path &path);
        const AssetRecord *GetAsset(GUID id);
        void Scan();
        void PreparePreview(const AssetType &type, const GUID &id);
        void RemovePreview(const AssetType &type, const GUID &id);
        void AddAsset(const AssetType &type, const directory_entry &entry);
        void UpdateAsset(const AssetType &type, const directory_entry &entry);
        void BuildIndex();
        bool Persist(const char *name = 0);
        bool IsValidDatabase();
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
        static std::unique_ptr<Project> m_projectInstance;
        path m_databasePath;
        std::map<AssetType, std::map<path, AssetRecord>> m_assetIndex;
        std::map<AssetType, std::string> m_assetTypeNames;
        std::set<std::string> m_modelExtensions;
        std::set<std::string> m_textureExtensions;
        std::map<AssetType, std::map<GUID, LPDIRECT3DTEXTURE9>> m_assetPreviews;
        ModelPreviewer m_modelPreviewer;
        ProjectRecord m_projectRecord;
    };
}

#endif
