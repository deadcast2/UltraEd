#include <filesystem>
#include <fstream>
#include "Debug.h"
#include "Project.h"
#include "PubSub.h"
#include "Util.h"

namespace UltraEd
{
    Project::Project(const char *name, const path &path, bool createDirectory) :
        m_name(name),
        m_databasePath(),
        m_assetIndex(),
        m_assetTypeNames({ { AssetType::Unknown, "unknown" }, { AssetType::Model, "model" }, { AssetType::Texture, "texture" } }),
        m_modelExtensions({ ".3ds", ".blend", ".fbx", ".dae", ".x", ".stl", ".wrl", ".obj" }),
        m_textureExtensions({ ".png", ".jpg", ".bmp", ".tga" })
    {
        if (!std::filesystem::exists(path))
        {
            Debug::Error("Failed to create new project since selected path doesn't exist.");
            return;
        }

        auto projectPath = createDirectory ? path / name : path;
        try
        {
            if (createDirectory)
            {
                std::filesystem::create_directory(projectPath);
            }
        }
        catch (const std::exception &e)
        {
            Debug::Error("Failed to create new project directory: " + std::string(e.what()));
        }

        m_databasePath = projectPath / "db.ultra";
        Save();

        if (std::filesystem::exists(m_databasePath))
        {
            Debug::Info("New project successfully created!");
        }
        else
        {
            Debug::Error("Error initializing new project database.");
        }

        Scan();

        PubSub::Subscribe({ "Activate", [&](void *data) {
            Scan();
        } });
    }

    void Project::Save()
    {
        json database = {
            { "project", {
                { "name", m_name },
                { "version", 0.1 }
            } }
        };

        for (const auto &name : m_assetTypeNames)
        {
            for (const auto &texture : m_assetIndex[name.first])
            {
                database[name.second].push_back(texture.second);
            }
        }

        std::ofstream file(m_databasePath, std::ofstream::out);
        if (file)
        {
            file << database.dump(1);
            file.close();
        }
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
        for (const auto &entry : recursive_directory_iterator(ParentPath()))
        {
            if (!IsValidFile(entry))
                continue;

            AssetType detectedType = DetectAssetType(entry.path());

            if (detectedType != AssetType::Unknown)
            {
                if (!FindAsset(detectedType, entry))
                {
                    AddAsset(detectedType, entry);
                    Debug::Info("Added " + m_assetTypeNames[detectedType] + ": " + entry.path().string());
                }
                else if (IsAssetModified(detectedType, entry))
                {
                    UpdateAsset(detectedType, entry);
                    Debug::Info("Updated " + m_assetTypeNames[detectedType] + ": " + entry.path().string());
                }
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
            { "sourcePath", entry.path().string().erase(0, ParentPath().string().size() + 1) }
        };

        UpdateAsset(type, entry);
    }

    void Project::UpdateAsset(const AssetType &type, const directory_entry &entry)
    {
        auto lastModified = entry.last_write_time().time_since_epoch().count();

        m_assetIndex[type][entry.path()]["lastModified"] = lastModified;

        InsertAsset(entry.path());
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

    bool Project::FindAsset(const AssetType &type, const path &path)
    {
        return m_assetIndex[type].find(path) != m_assetIndex[type].end();
    }

    bool Project::InsertAsset(const path &path)
    {
        if (!InitializeLibrary())
            return false;

        try
        {
            copy(path, LibraryPath(), copy_options::overwrite_existing);
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
}
