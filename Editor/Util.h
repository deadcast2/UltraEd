#ifndef _UTIL_H_
#define _UTIL_H_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <functional>
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
        static boost::uuids::uuid NewUuid();
        static boost::uuids::uuid StringToUuid(const std::string &uuid);
        static std::string UuidToString(const boost::uuids::uuid &uuid);
        static std::string NewResourceName(int count);
        static std::string ReplaceString(const std::string &str, const std::string &from, const std::string &to);
        static std::vector<std::string> SplitString(const std::string &str, const std::string &delimiter);
        static void ToFloat3(const D3DXVECTOR3 &vec, float *position);
        static std::string ToLower(const std::string &str);

    private:
        Util() {};
    };
}

#endif
