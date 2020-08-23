#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <map>
#include <nlohmann/json.hpp>
#include <set>

using json = nlohmann::json;

namespace UltraEd
{
    class Project
    {
    public:
        Project(const char *name, const std::filesystem::path &path, bool createDirectory);

    private:
        std::filesystem::path ParentPath();
        void Save();
        void Scan();
        void AddTexture(const std::filesystem::directory_entry &entry);
        bool IsTextureModified(const std::filesystem::directory_entry &entry);
        bool FindAsset(const std::string &type, const std::filesystem::path &path);

    private:
        const char *m_name;
        std::filesystem::path m_databasePath;
        std::map<std::filesystem::path, json> m_textureIndex;
        std::set<std::string> m_textureExtensions;
    };
}

#endif
