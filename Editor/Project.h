#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace UltraEd
{
    class Project
    {
    public:
        Project(const char *name, const std::filesystem::path &path, bool createDirectory);

    private:
        json m_database;
    };
}

#endif
