#include <filesystem>
#include "Debug.h"
#include "Project.h"

namespace UltraEd
{
    Project::Project(const char *name, const std::filesystem::path &path, bool createDirectory)
    {
        if (!std::filesystem::exists(path))
        {
            Debug::Error("Failed to create new project since selected path doesn't exist.");
            return;
        }

        if (createDirectory)
        {
            if (!std::filesystem::create_directory(path / name))
            {
                Debug::Error("Failed to create new project directory.");
                return;
            }
        }
    }
}
