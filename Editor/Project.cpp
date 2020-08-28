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
        m_modelExtensions({ ".3ds", ".blend", ".fbx", ".dae", ".x", ".stl", ".wrl", ".obj" }),
        m_textureExtensions({ ".png", ".jpg", ".bmp", ".tga" })
    {
        PubSub::Subscribe({ "Activate", [&](void *data) {
            Scan();
        } });
    }

    Project::Project(const path &path) : Project()
    {
        m_databasePath = path / "db.ultra";

        if (!exists(m_databasePath))
        {
            Debug::Error("Failed to load project since it doesn't seem to exist.");
            return;
        }

        std::ifstream file(m_databasePath);
        if (file)
        {
            std::stringstream stream;
            stream << file.rdbuf();
            json database = json::parse(stream.str());

            if (!IsValidDatabase(database))
            {
                Debug::Error("Failed to load project since it doesn't seem to be valid.");
                return;
            }

            Load(database);
        }

        Scan();
    }

    Project::Project(const char *name, const path &path, bool createDirectory) : Project()
    {
        if (!exists(path))
        {
            Debug::Error("Failed to create new project since selected path doesn't exist.");
            return;
        }

        const auto projectPath = createDirectory ? path / name : path;
        try
        {
            if (createDirectory)
            {
                create_directory(projectPath);
            }
        }
        catch (const std::exception &e)
        {
            Debug::Error("Failed to create new project directory: " + std::string(e.what()));
        }

        m_name = name;
        m_databasePath = projectPath / "db.ultra";

        if (Save() && exists(m_databasePath))
        {
            Debug::Info("New project successfully created!");
        }
        else
        {
            Debug::Error("Error initializing new project database.");
        }

        Scan();
    }

    bool Project::Save()
    {
        json database = {
            { "project", {
                { "name", m_name },
            { "version", 0.1 }
        } }
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

    path Project::ParentPath()
    {
        return m_databasePath.parent_path();
    }

    path Project::LibraryPath()
    {
        return ParentPath() / "Library";
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
            if (name.first == AssetType::Unknown)
                continue;

            for (auto asset : database[name.second])
            {
                asset["purgeId"] = nullptr;
                m_assetIndex[name.first][ParentPath() / asset["sourcePath"].get<std::string>()] = asset;
            }
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
            auto id = static_cast<std::string>(m_assetIndex[type][path]["id"]);
            copy(path, LibraryPath() / id, copy_options::overwrite_existing);
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
            std::vector<path> assetsToRemove;

            for (const auto &asset : m_assetIndex[name.first])
            {
                if (asset.second["purgeId"] != Util::GuidToString(purgeId))
                {
                    assetsToRemove.push_back(asset.first);
                    Debug::Warning("Removed " + name.second + ": " + asset.first.string());
                }
            }

            for (const auto &asset : assetsToRemove)
            {
                m_assetIndex[name.first].erase(asset);
            }
        }
    }
}
