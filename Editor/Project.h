#ifndef _PROJECT_H_
#define _PROJECT_H_

namespace UltraEd
{
    class Project
    {
    public:
        Project(const char *name, const std::filesystem::path &path, bool createDirectory);
    };
}

#endif
