#ifndef _REGISTRY_H_
#define _REGISTRY_H_

#include <windows.h>
#include <string>

#define REG_DATA_LENGTH SIZE_MAX

using namespace std;

namespace UltraEd
{
    class Registry
    {
    public:
        static bool Set(const string &key, const string &value);
        static bool Get(const string &key, string &value);

    private:
        static const string m_key;
    };
}

#endif
