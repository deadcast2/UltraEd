#include <filesystem>
#include <fstream>
#include <istream>
#include <sstream>
#include "Converters.h"
#include "Debug.h"
#include "Project.h"
#include "Util.h"

namespace UltraEd
{
    std::unique_ptr<Project> Project::m_projectInstance = NULL;

    void Project::Activate(std::vector<boost::uuids::uuid> *updatedAssetIds)
    {
        if (IsLoaded())
            m_projectInstance->Scan(updatedAssetIds);
    }

    void Project::New(const char *name, const path &path, bool createDirectory)
    {
        m_projectInstance = std::make_unique<Project>(m_constructor_tag {}, name, path, createDirectory);
    }

    void Project::Load(const path &path)
    {
        m_projectInstance = std::make_unique<Project>(m_constructor_tag {}, path);
    }

    bool Project::Save(const char *name)
    {
        if (IsLoaded())
            return m_projectInstance->Persist(name);
        return false;
    }

    bool Project::IsLoaded()
    {
        return m_projectInstance != NULL;
    }

    std::map<boost::uuids::uuid, LPDIRECT3DTEXTURE9> Project::Previews(const AssetType &type, LPDIRECT3DDEVICE9 device)
    {
        if (!IsLoaded())
            return std::map<boost::uuids::uuid, LPDIRECT3DTEXTURE9>();

        for (auto &preview : m_projectInstance->m_assetPreviews[type])
        {
            if (preview.second != NULL)
                continue;

            auto asset = m_projectInstance->GetAsset(preview.first);
            if (asset == NULL)
                continue;

            switch (type)
            {
                case AssetType::Texture:
                    D3DXCreateTextureFromFile(device,
                        m_projectInstance->LibraryPath(asset).string().c_str(), &preview.second);
                    break;
                case AssetType::Model:
                    m_projectInstance->m_modelPreviewer.Render(device,
                        m_projectInstance->LibraryPath(asset).string().c_str(), &preview.second);
                    break;
            }
        }

        return m_projectInstance->m_assetPreviews[type];
    }

    path Project::GetAssetPath(const boost::uuids::uuid &id)
    {
        if (IsLoaded())
        {
            auto asset = m_projectInstance->GetAsset(id);
            if (asset)
            {
                return m_projectInstance->LibraryPath(asset);
            }
        }
        return path();
    }

    path Project::BuildPath()
    {
        if (IsLoaded())
        {
            auto path = m_projectInstance->LibraryPath() / "Build";
            if (m_projectInstance->InitializePath(path))
                return path;
        }

        return path();
    }

    void Project::Refresh()
    {
        if (IsLoaded())
        {
            m_projectInstance->BuildIndex();
        }
    }

    Project::Project(m_constructor_tag tag) :
        m_databasePath(),
        m_assetIndex(),
        m_assetTypeNames({ { AssetType::Unknown, "unknown" }, { AssetType::Model, "model" }, { AssetType::Texture, "texture" } }),
        m_modelExtensions({ ".3ds", ".fbx", ".dae", ".x", ".stl", ".wrl", ".obj" }),
        m_textureExtensions({ ".png", ".jpg", ".bmp", ".tga" }),
        m_assetPreviews(),
        m_modelPreviewer(),
        m_projectRecord()
    { }

    Project::Project(m_constructor_tag tag, const char *name, const path &path, bool createDirectory) : Project(tag)
    {
        if (!exists(path))
        {
            throw std::exception("Failed to create new project since selected path doesn't exist.");
        }

        const auto projectPath = createDirectory ? path / name : path;
        if (createDirectory)
        {
            create_directory(projectPath);
        }

        m_databasePath = projectPath / "db.ultra";

        if (Save(name) && exists(m_databasePath))
        {
            Debug::Instance().Info("New project successfully created!");
        }
        else
        {
            throw std::exception("Error initializing new project database.");
        }

        Scan();
    }

    Project::Project(m_constructor_tag tag, const path &path) : Project(tag)
    {
        m_databasePath = path / "db.ultra";

        if (!exists(m_databasePath))
        {
            throw std::exception("Failed to load project since it doesn't seem to exist.");
        }

        std::ifstream file(m_databasePath);
        if (file)
        {
            std::stringstream stream;
            stream << file.rdbuf();
            m_projectRecord = json::parse(stream.str());

            if (!IsValidDatabase())
            {
                throw std::exception("Failed to load project since it doesn't seem to be valid.");
            }

            BuildIndex();
        }

        Scan();
    }

    Project::~Project()
    {
        // Will release all preview assets.
        BuildIndex();
    }

    bool Project::Persist(const char *name)
    {
        // Use existing project name when no name is passed.
        m_projectRecord = ProjectRecord({ name ? name : m_projectRecord.name, 1 });

        for (const auto &name : m_assetTypeNames)
        {
            for (const auto &asset : m_assetIndex[name.first])
            {
                m_projectRecord.assets.push_back(asset.second);
            }
        }

        std::ofstream file(m_databasePath);
        if (file)
        {
            json database = m_projectRecord;
            file << database.dump(1);
            return true;
        }

        return false;
    }

    path Project::ParentPath()
    {
        return m_databasePath.parent_path();
    }

    path Project::LibraryPath(const AssetRecord *record)
    {
        auto path = ParentPath() / "Library";
        if (record != NULL)
        {
            auto ext = ::path(record->sourcePath).extension().string();
            return path / (Util::UuidToString(record->id) + ext);
        }
        return path;
    }

    void Project::Scan(std::vector<boost::uuids::uuid> *updatedAssetIds)
    {
        auto purgeId = Util::NewUuid();

        for (const auto &entry : recursive_directory_iterator(ParentPath()))
        {
            if (!IsValidFile(entry))
                continue;

            AssetType detectedType = DetectAssetType(entry.path());

            if (detectedType != AssetType::Unknown)
            {
                if (!AssetExists(detectedType, entry))
                {
                    AddAsset(detectedType, entry);
                    Debug::Instance().Info("Added " + m_assetTypeNames[detectedType] + ": " + entry.path().string());
                }
                else if (IsAssetModified(detectedType, entry))
                {
                    auto assetRecord = UpdateAsset(detectedType, entry);
                    if (updatedAssetIds) updatedAssetIds->push_back(assetRecord->id);
                    Debug::Instance().Info("Updated " + m_assetTypeNames[detectedType] + ": " + entry.path().string());
                }

                VerifyAsset(purgeId, detectedType, entry);
            }
        }

        PurgeMissingAssets(purgeId);
    }

    void Project::PreparePreview(const AssetType &type, const boost::uuids::uuid &id)
    {
        RemovePreview(type, id);

        m_assetPreviews[type][id] = 0;
    }

    void Project::RemovePreview(const AssetType &type, const boost::uuids::uuid &id)
    {
        if (m_assetPreviews[type].find(id) != m_assetPreviews[type].end())
        {
            if (m_assetPreviews[type][id] != NULL)
                m_assetPreviews[type][id]->Release();

            m_assetPreviews[type].erase(id);
        }
    }

    AssetType Project::DetectAssetType(const path &path)
    {
        if (IsSupportedModel(path))
            return AssetType::Model;
        else if (IsSupportedTexture(path))
            return AssetType::Texture;

        return AssetType::Unknown;
    }

    const AssetRecord *Project::GetAsset(const boost::uuids::uuid &id)
    {
        for (const auto &name : m_assetTypeNames)
        {
            for (const auto &asset : m_assetIndex[name.first])
            {
                if (asset.second.id == id)
                    return &asset.second;
            }
        }

        return NULL;
    }

    void Project::AddAsset(const AssetType &type, const directory_entry &entry)
    {
        m_assetIndex[type][entry.path()] = {
            Util::NewUuid(),
            type,
            Util::NewUuid(),
            entry.path().string().erase(0, ParentPath().string().size() + 1),
            0
        };

        UpdateAsset(type, entry);
    }

    const AssetRecord *Project::UpdateAsset(const AssetType &type, const directory_entry &entry)
    {
        auto lastModified = entry.last_write_time().time_since_epoch().count();
        auto assetRecord = &m_assetIndex[type][entry.path()];

        assetRecord->lastModified = lastModified;
        InsertAsset(type, entry.path());
        PreparePreview(type, assetRecord->id);

        return assetRecord;
    }

    void Project::BuildIndex()
    {
        m_assetIndex.clear();

        for (const auto &asset : m_projectRecord.assets)
        {
            m_assetIndex[asset.type][ParentPath() / asset.sourcePath] = asset;
            PreparePreview(asset.type, asset.id);
        }
    }

    bool Project::IsValidDatabase()
    {
        return !m_projectRecord.name.empty() && m_projectRecord.version > 0;
    }

    bool Project::IsValidFile(const directory_entry &entry)
    {
        if (!entry.is_regular_file())
            return false;

        // Verify the file is not within the project's library folder.
        auto filePath = entry.path();
        while (filePath.has_parent_path() && filePath != filePath.root_path())
        {
            if (filePath.parent_path() == LibraryPath())
                return false;

            filePath = filePath.parent_path();
        }

        return true;
    }

    bool Project::IsSupportedModel(const path &path)
    {
        return m_modelExtensions.find(Util::ToLower(path.extension().string())) != m_modelExtensions.end();
    }

    bool Project::IsSupportedTexture(const path &path)
    {
        return m_textureExtensions.find(Util::ToLower(path.extension().string())) != m_textureExtensions.end();
    }

    bool Project::IsAssetModified(const AssetType &type, const directory_entry &entry)
    {
        auto assetRecord = m_assetIndex[type][entry.path()];
        bool isModified = assetRecord.lastModified != entry.last_write_time().time_since_epoch().count();
        bool isMissing = !exists(LibraryPath(&assetRecord));

        return isModified || isMissing;
    }

    bool Project::AssetExists(const AssetType &type, const path &path)
    {
        return m_assetIndex[type].find(path) != m_assetIndex[type].end();
    }

    bool Project::InsertAsset(const AssetType &type, const path &path)
    {
        if (!InitializePath(LibraryPath()))
            return false;

        try
        {
            copy_file(path, LibraryPath(&m_assetIndex[type][path]), copy_options::overwrite_existing);
        }
        catch (const std::exception &e)
        {
            Debug::Instance().Error("Failed to copy file: " + std::string(e.what()));
            return false;
        }

        return exists(LibraryPath() / path.filename());
    }

    bool Project::InitializePath(const path &path)
    {
        if (!exists(path))
        {
            try
            {
                create_directory(path);
            }
            catch (const std::exception &e)
            {
                Debug::Instance().Error("Failed to create directory: " + std::string(e.what()));
                return false;
            }
        }

        return true;
    }

    void Project::VerifyAsset(const boost::uuids::uuid &purgeId, const AssetType &type, const directory_entry &entry)
    {
        m_assetIndex[type][entry.path()].purgeId = purgeId;
    }

    void Project::PurgeMissingAssets(const boost::uuids::uuid &purgeId)
    {
        for (const auto &name : m_assetTypeNames)
        {
            std::vector<std::pair<path, AssetRecord>> assetsToRemove;

            for (const auto &asset : m_assetIndex[name.first])
            {
                if (asset.second.purgeId != purgeId)
                {
                    assetsToRemove.push_back(asset);
                    Debug::Instance().Warning("Removed " + name.second + ": " + asset.first.string());
                }
            }

            for (const auto &asset : assetsToRemove)
            {
                m_assetIndex[name.first].erase(asset.first);
                RemovePreview(name.first, asset.second.id);

                try
                {
                    remove(LibraryPath(&asset.second));
                }
                catch (const std::exception &e)
                {
                    Debug::Instance().Error("Failed to remove asset: " + std::string(e.what()));
                }
            }
        }
    }
}
