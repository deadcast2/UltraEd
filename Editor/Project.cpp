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
    Project::Project() :
        m_name(),
        m_databasePath(),
        m_assetIndex(),
        m_assetTypeNames({ { AssetType::Unknown, "unknown" }, { AssetType::Model, "model" }, { AssetType::Texture, "texture" } }),
        m_modelExtensions({ ".3ds", ".fbx", ".dae", ".x", ".stl", ".wrl", ".obj" }),
        m_textureExtensions({ ".png", ".jpg", ".bmp", ".tga" }),
        m_activateSubscriber(),
        m_assetPreviews(),
        m_modelPreviewer()
    {
        m_activateSubscriber = PubSub::Subscribe({ "Activate", [&](void *data) {
            Scan();
        } });
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
            json database = json::parse(stream.str());

            if (!IsValidDatabase(database))
            {
                throw std::exception("Failed to load project since it doesn't seem to be valid.");
            }

            Load(database);
        }

        Scan();
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

        m_name = name;
        m_databasePath = projectPath / "db.ultra";

        if (Save() && exists(m_databasePath))
        {
            Debug::Info("New project successfully created!");
        }
        else
        {
            throw std::exception("Error initializing new project database.");
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

    bool Project::Save()
    {
        json database = {
            { "project", {
                { "name", m_name },
                { "version", 0.1 }
            }}
        };

        for (const auto &name : m_assetTypeNames)
        {
            for (const auto &asset : m_assetIndex[name.first])
            {
                database[name.second].push_back(asset.second);
            }
        }

        std::ofstream file(m_databasePath);
        if (file)
        {
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

    path Project::LibraryPath(const json &asset)
    {
        auto path = ParentPath() / "Library";
        if (asset != NULL)
        {
            auto id = static_cast<std::string>(asset["id"]);
            auto sourcePath = static_cast<std::string>(asset["sourcePath"]);
            auto ext = ::path(sourcePath).extension().string();
            return path / (id + ext);
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

    void Project::Load(const json &database)
    {
        m_name = database["project"]["name"];

        for (const auto &name : m_assetTypeNames)
        {
            if (!database.contains(name.second))
                continue;

            for (auto asset : database[name.second])
            {
                asset["purgeId"] = nullptr;
                m_assetIndex[name.first][ParentPath() / asset["sourcePath"].get<std::string>()] = asset;
                PreparePreview(name.first, Util::StringToGuid(asset["id"].get<std::string>().c_str()));
            }
        }
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

    json Project::GetAsset(GUID id)
    {
        auto idString = Util::GuidToString(id);

        for (const auto &name : m_assetTypeNames)
        {
            for (const auto &asset : m_assetIndex[name.first])
            {
                if (asset.second["id"].get<std::string>() == idString)
                    return asset.second;
            }
        }

        return NULL;
    }

    void Project::AddAsset(const AssetType &type, const directory_entry &entry)
    {
        m_assetIndex[type][entry.path()] = {
            { "id", Util::GuidToString(Util::NewGuid()) },
            { "sourcePath", entry.path().string().erase(0, ParentPath().string().size() + 1) },
            { "purgeId", nullptr }
        };

        UpdateAsset(type, entry);
    }

    void Project::UpdateAsset(const AssetType &type, const directory_entry &entry)
    {
        auto lastModified = entry.last_write_time().time_since_epoch().count();

        m_assetIndex[type][entry.path()]["lastModified"] = lastModified;

        InsertAsset(type, entry.path());
        PreparePreview(type, Util::StringToGuid(m_assetIndex[type][entry.path()]["id"].get<std::string>().c_str()));
    }

    bool Project::IsValidDatabase(const json &database)
    {
        auto name = database["project"]["name"];
        auto version = database["project"]["version"];

        return !name.is_null() && !version.is_null();
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
        return m_assetIndex[type][entry.path()]["lastModified"] != entry.last_write_time().time_since_epoch().count();
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
            copy(path, LibraryPath(m_assetIndex[type][path]), copy_options::overwrite_existing);
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
        m_assetIndex[type][entry.path()]["purgeId"] = Util::GuidToString(purgeId);
    }

    void Project::PurgeMissingAssets(const GUID &purgeId)
    {
        for (const auto &name : m_assetTypeNames)
        {
            std::vector<std::pair<path, json>> assetsToRemove;

            for (const auto &asset : m_assetIndex[name.first])
            {
                if (asset.second["purgeId"] != Util::GuidToString(purgeId))
                {
                    assetsToRemove.push_back(asset);
                    Debug::Warning("Removed " + name.second + ": " + asset.first.string());
                }
            }

            for (const auto &asset : assetsToRemove)
            {
                m_assetIndex[name.first].erase(asset.first);
                RemovePreview(name.first, Util::StringToGuid(asset.second["id"].get<std::string>().c_str()));

                try
                {
                    remove(LibraryPath(asset));
                }
                catch (const std::exception &e)
                {
                    Debug::Error("Failed to remove asset: " + std::string(e.what()));
                }
            }
        }
    }
}
