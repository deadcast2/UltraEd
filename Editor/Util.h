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
        static D3DXVECTOR3 ToEuler(const D3DXMATRIX &rotation);
        static void CopyBackBuffer(UINT width, UINT height, LPDIRECT3DDEVICE9 source,
            LPDIRECT3DDEVICE9 target, LPDIRECT3DTEXTURE9 *texture);
        static float Snap(float value);
        static float GetDPIScale();
        static bool IntersectTriangle(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &v0,
            const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2, float *dist);
        static void ScreenRaycast(LPDIRECT3DDEVICE9 device, const D3DXVECTOR2 &screenPoint, const D3DXMATRIX &view, 
            D3DXVECTOR3 *origin, D3DXVECTOR3 *dir);

    private:
        Util() {};
    };
}

#endif
