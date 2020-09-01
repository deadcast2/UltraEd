#include <filesystem>
#include <fstream>
#include <istream>
#include <sstream>
#include "Debug.h"
#include "Project.h"
#include "PubSub.h"
#include "Util.h"

namespace UltraEd
{
    void to_json(json &j, const AssetRecord &a)
    {
        j = json {
            { "id", Util::GuidToString(a.id) },
            { "type", a.type },
            { "purgeId", Util::GuidToString(a.purgeId) },
            { "sourcePath", a.sourcePath },
            { "lastModified", a.lastModified }
        };
    }

    void from_json(const json &j, AssetRecord &a)
    {
        a.id = Util::StringToGuid(j.at("id").get<std::string>().c_str());
        a.purgeId = Util::StringToGuid(j.at("purgeId").get<std::string>().c_str());
        j.at("type").get_to(a.type);
        j.at("sourcePath").get_to(a.sourcePath);
        j.at("lastModified").get_to(a.lastModified);
    }

    void to_json(json &j, const ProjectRecord &p)
    {
        j = json {
            { "name", p.name },
            { "version", p.version },
            { "assets", p.assets }
        };
    }

    void from_json(const json &j, ProjectRecord &p)
    {
        j.at("name").get_to(p.name);
        j.at("version").get_to(p.version);
        j.at("assets").get_to(p.assets);
    }

    Project::Project() :
        m_databasePath(),
        m_assetIndex(),
        m_assetTypeNames({ { AssetType::Unknown, "unknown" }, { AssetType::Model, "model" }, { AssetType::Texture, "texture" } }),
        m_modelExtensions({ ".3ds", ".fbx", ".dae", ".x", ".stl", ".wrl", ".obj" }),
        m_textureExtensions({ ".png", ".jpg", ".bmp", ".tga" }),
        m_activateSubscriber(),
        m_assetPreviews(),
        m_modelPreviewer(),
        m_projectRecord()
    {
        m_activateSubscriber = PubSub::Subscribe({ "Activate", [&](void *data) {
            Scan();
        } });
    }

    Project::Project(const char *name, const path &path, bool createDirectory) : Project()
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
            Debug::Info("New project successfully created!");
        }
        else
        {
            throw std::exception("Error initializing new project database.");
        }

        Scan();
    }

    Project::Project(const path &path) : Project()
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
        if (m_activateSubscriber)
            m_activateSubscriber();

        for (const auto &name : m_assetTypeNames)
        {
            for (auto &assetPreview : m_assetPreviews[name.first])
            {
                if (assetPreview.second != NULL)
                    assetPreview.second->Release();
            }
        }
    }

    bool Project::Save(const char *name)
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

    const std::map<GUID, LPDIRECT3DTEXTURE9> Project::Textures(LPDIRECT3DDEVICE9 device)
    {
        for (auto &texture : m_assetPreviews[AssetType::Texture])
        {
            if (texture.second == NULL)
            {
                auto asset = GetAsset(texture.first);
                if (asset == NULL) continue;

                D3DXCreateTextureFromFile(device, LibraryPath(asset).string().c_str(), &texture.second);
            }
        }

        return m_assetPreviews[AssetType::Texture];
    }

    const std::map<GUID, LPDIRECT3DTEXTURE9> Project::Models(LPDIRECT3DDEVICE9 device)
    {
        for (auto &model : m_assetPreviews[AssetType::Model])
        {
            if (model.second == NULL)
            {
                auto asset = GetAsset(model.first);
                if (asset == NULL) continue;

                m_modelPreviewer.Render(device, LibraryPath(asset).string().c_str(), &model.second);
            }
        }

        return m_assetPreviews[AssetType::Model];
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
            return path / (Util::GuidToString(record->id) + ext);
        }
        return path;
    }

    void Project::Scan()
    {
        auto purgeId = Util::NewGuid();

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
                    Debug::Info("Added " + m_assetTypeNames[detectedType] + ": " + entry.path().string());
                }
                else if (IsAssetModified(detectedType, entry))
                {
                    UpdateAsset(detectedType, entry);
                    Debug::Info("Updated " + m_assetTypeNames[detectedType] + ": " + entry.path().string());
                }

                VerifyAsset(purgeId, detectedType, entry);
            }
        }

        PurgeMissingAssets(purgeId);
    }

    void Project::PreparePreview(const AssetType &type, const GUID &id)
    {
        RemovePreview(type, id);

        m_assetPreviews[type][id] = 0;
    }

    void Project::RemovePreview(const AssetType &type, const GUID &id)
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

    const AssetRecord *Project::GetAsset(GUID id)
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
            Util::NewGuid(),
            type,
            Util::NewGuid(),
            entry.path().string().erase(0, ParentPath().string().size() + 1),
            0
        };

        UpdateAsset(type, entry);
    }

    void Project::UpdateAsset(const AssetType &type, const directory_entry &entry)
    {
        auto lastModified = entry.last_write_time().time_since_epoch().count();

        m_assetIndex[type][entry.path()].lastModified = lastModified;

        InsertAsset(type, entry.path());
        PreparePreview(type, m_assetIndex[type][entry.path()].id);
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
        return entry.path().parent_path() != LibraryPath() && entry.is_regular_file();
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
        return m_assetIndex[type][entry.path()].lastModified != entry.last_write_time().time_since_epoch().count();
    }

    bool Project::AssetExists(const AssetType &type, const path &path)
    {
        return m_assetIndex[type].find(path) != m_assetIndex[type].end();
    }

    bool Project::InsertAsset(const AssetType &type, const path &path)
    {
        if (!InitializeLibrary())
            return false;

        try
        {
            copy(path, LibraryPath(&m_assetIndex[type][path]), copy_options::overwrite_existing);
        }
        catch (const std::exception &e)
        {
            Debug::Error("Failed to copy file: " + std::string(e.what()));
            return false;
        }

        return exists(LibraryPath() / path.filename());
    }

    bool Project::InitializeLibrary()
    {
        if (!exists(LibraryPath()))
        {
            try
            {
                create_directory(LibraryPath());
            }
            catch (const std::exception &e)
            {
                Debug::Error("Failed to create library directory: " + std::string(e.what()));
                return false;
            }
        }

        return true;
    }

    void Project::VerifyAsset(const GUID &purgeId, const AssetType &type, const directory_entry &entry)
    {
        m_assetIndex[type][entry.path()].purgeId = purgeId;
    }

    void Project::PurgeMissingAssets(const GUID &purgeId)
    {
        for (const auto &name : m_assetTypeNames)
        {
            std::vector<std::pair<path, AssetRecord>> assetsToRemove;

            for (const auto &asset : m_assetIndex[name.first])
            {
                if (asset.second.purgeId != purgeId)
                {
                    assetsToRemove.push_back(asset);
                    Debug::Warning("Removed " + name.second + ": " + asset.first.string());
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
                    Debug::Error("Failed to remove asset: " + std::string(e.what()));
                }
            }
        }
    }
}
