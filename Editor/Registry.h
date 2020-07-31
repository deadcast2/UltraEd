#ifndef _REGISTRY_H_
#define _REGISTRY_H_

#include <windows.h>
#include <string>

#define REG_DATA_LENGTH SIZE_MAX

namespace UltraEd
{
    class Registry
    {
    public:
        static bool Set(const std::string &key, const std::string &value);
        static bool Get(const std::string &key, std::string &value);

    private:
        static const std::string m_key;
    };
}

#endif
