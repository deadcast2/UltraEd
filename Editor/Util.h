#ifndef _UTIL_H_
#define _UTIL_H_

#define CLSID_LENGTH 40

#include <functional>
#include <rpc.h>
#include <string>
#include <vector>
#include <memory>
#include <d3dx9.h>

namespace UltraEd
{
    class Util
    {
    public:
        static float Lerp(float time, float start, float end);
        static GUID NewGuid();
        static GUID StringToGuid(const char *guid);
        static std::string GuidToString(GUID guid);
        static std::string RootPath();
        static std::string NewResourceName(int count);
        static char *ReplaceString(const char *str, const char *from, const char *to);
        static std::vector<std::string> SplitString(const char *str, const char delimiter);
        static void ToFloat3(const D3DXVECTOR3 &vec, float *position);

    private:
        Util() {};
    };
}

#endif
