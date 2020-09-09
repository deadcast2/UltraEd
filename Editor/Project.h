#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <d3d9.h>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
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
        static void Activate();
        static void New(const char *name, const path &path, bool createDirectory);
        static void Load(const path &path);
        static bool Save(const char *name = 0);
        static bool IsLoaded();
        static std::map<boost::uuids::uuid, LPDIRECT3DTEXTURE9> Previews(const AssetType &type, LPDIRECT3DDEVICE9 device);
        static path GetAssetPath(const boost::uuids::uuid &id);
        static path BuildPath();
    
    private:
        path ParentPath();
        path LibraryPath(const AssetRecord *record = 0);
        AssetType DetectAssetType(const path &path);
        const AssetRecord *GetAsset(const boost::uuids::uuid &id);
        void Scan();
        void PreparePreview(const AssetType &type, const boost::uuids::uuid &id);
        void RemovePreview(const AssetType &type, const boost::uuids::uuid &id);
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
        bool InitializePath(const path &path);
        void VerifyAsset(const boost::uuids::uuid &purgeId, const AssetType &type, const directory_entry &entry);
        void PurgeMissingAssets(const boost::uuids::uuid &purgeId);

    private:
        static std::unique_ptr<Project> m_projectInstance;
        path m_databasePath;
        std::map<AssetType, std::map<path, AssetRecord>> m_assetIndex;
        std::map<AssetType, std::string> m_assetTypeNames;
        std::set<std::string> m_modelExtensions;
        std::set<std::string> m_textureExtensions;
        std::map<AssetType, std::map<boost::uuids::uuid, LPDIRECT3DTEXTURE9>> m_assetPreviews;
        ModelPreviewer m_modelPreviewer;
        ProjectRecord m_projectRecord;
    };
}

#endif
