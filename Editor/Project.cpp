#include <filesystem>
#include <fstream>
#include "Debug.h"
#include "Project.h"
#include "PubSub.h"
#include "Util.h"

namespace UltraEd
{
    Project::Project(const char *name, const std::filesystem::path &path, bool createDirectory) :
        m_name(name),
        m_databasePath(),
        m_textureIndex(),
        m_textureExtensions({ ".png", ".jpg" })
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
            } },
            { "textures", json::array() }
        };

        std::ofstream file(m_databasePath, std::ofstream::out);
        if (file)
        {
            file << database.dump(1);
            file.close();
        }
    }

    std::filesystem::path Project::ParentPath()
    {
        return m_databasePath.parent_path();
    }

    void Project::Scan()
    {
        for (const auto &entry : std::filesystem::recursive_directory_iterator(ParentPath()))
        {
            if (!entry.is_regular_file())
                continue;

            if (m_textureExtensions.find(Util::ToLower(entry.path().extension().string())) != m_textureExtensions.end())
            {
                if (!FindAsset("texture", entry))
                {
                    AddTexture(entry);
                    Debug::Info("Added new texture: " + entry.path().filename().string());
                }
                else
                {
                    if (IsTextureModified(entry))
                    {
                        m_textureIndex[entry.path()]["lastModified"] = entry.last_write_time().time_since_epoch().count();
                        Debug::Info("Modified texture: " + entry.path().filename().string());
                    }
                }
            }
        }
    }

    void Project::AddTexture(const std::filesystem::directory_entry &entry)
    {
        auto lastModified = entry.last_write_time().time_since_epoch().count();

        m_textureIndex[entry.path()] = {
            { "id", Util::GuidToString(Util::NewGuid()) },
            { "sourcePath", entry.path().string().erase(0, ParentPath().string().size() + 1) },
            { "lastModified", lastModified }
        };
    }

    bool Project::IsTextureModified(const std::filesystem::directory_entry &entry)
    {
        return m_textureIndex[entry.path()]["lastModified"] != entry.last_write_time().time_since_epoch().count();
    }

    bool Project::FindAsset(const std::string &type, const std::filesystem::path &path)
    {
        return m_textureIndex.find(path) != m_textureIndex.end();
    }
}
