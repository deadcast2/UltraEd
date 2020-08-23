#include <filesystem>
#include <fstream>
#include "Debug.h"
#include "Project.h"

namespace UltraEd
{
    Project::Project(const char *name, const std::filesystem::path &path, bool createDirectory) :
        m_database()
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

        m_database = {
            { "project", {
                { "name", name },
                { "version", 0.1 }
            }}
        };

        auto dbPath = projectPath / "db.ultra";
        std::ofstream file(dbPath, std::ofstream::out);
        if (!file)
        {
            file << m_database.dump(1);
            file.close();
        }

        if (std::filesystem::exists(dbPath))
        {
            Debug::Info("New project successfully created!");
        }
        else
        {
            Debug::Error("Error initializing new project database.");
        }
    }
}
