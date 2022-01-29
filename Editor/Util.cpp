#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fstream>
#include <sstream>
#include <regex>
#include "Util.h"
#include "shlwapi.h"

namespace UltraEd
{
    float Util::Lerp(float time, float start, float end)
    {
        return (1 - time) * start + time * end;
    }

    boost::uuids::uuid Util::NewUuid()
    {
        boost::uuids::random_generator gen;
        return gen();
    }

    boost::uuids::uuid Util::StringToUuid(const std::string &uuid)
    {
        auto gen = boost::uuids::string_generator();
        return gen(uuid);
    }

    std::string Util::UuidToString(const boost::uuids::uuid &uuid)
    {
        return boost::uuids::to_string(uuid);
    }

    std::string Util::NewResourceName(int count)
    {
        return std::string("UER_").append(std::to_string(count));
    }

    std::string Util::UniqueName(std::string name)
    {
        // Will append a small portion of a unique uuid's chars and replace an existing partial uuid postfix.
        auto replaced = std::regex_replace(name, std::regex("-[abcdefABCDEF0123456789]{5}$"), "");

        return std::string(replaced).append("-").append(UuidToString(NewUuid()).substr(0, 5));
    }

    std::string Util::ReplaceString(const std::string &str, const std::string &from, const std::string &to)
    {
        std::string strCopy(str);
        boost::replace_all(strCopy, from, to);
        return strCopy;
    }

    std::vector<std::string> Util::SplitString(const std::string &str, const std::string &delimiter)
    {
        std::vector<std::string> tokens;
        boost::algorithm::split(tokens, str, boost::algorithm::is_any_of(delimiter));
        return tokens;
    }

    std::string Util::GetPathFor(const std::string &name)
    {
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            std::string path(buffer);
            return path.append("\\..\\..\\..\\").append(name);
        }
        return std::string();
    }

    std::string Util::GetSnippet(const std::string &fileName)
    {
        std::string snippet;
        std::ifstream file(GetPathFor(std::string("Engine\\Snippets\\").append(fileName)), std::ios::in);
        
        if (file)
        {
            std::ostringstream buffer;
            buffer << file.rdbuf();
            snippet = buffer.str();

            file.close();
        }

        return snippet;
    }

    void Util::ToFloat3(const D3DXVECTOR3 &vec, float *position)
    {
        if (position != NULL)
        {
            position[0] = vec.x;
            position[1] = vec.y;
            position[2] = vec.z;
        }
    }

    std::string Util::ToLower(const std::string &str)
    {
        std::string strCopy(str);
        boost::to_lower(strCopy);
        return strCopy;
    }

    D3DXVECTOR3 Util::ToEuler(const D3DXMATRIX &rotation)
    {
        D3DXVECTOR3 euler;

        euler.x = asinf(-rotation._32);

        if (cosf(euler.x) > 0.0001f)
        {
            euler.y = atan2f(rotation._31, rotation._33);
            euler.z = atan2f(rotation._12, rotation._22);
        }
        else
        {
            euler.y = 0.0f;
            euler.z = atan2f(-rotation._21, rotation._11);
        }

        return D3DXVECTOR3(D3DXToDegree(euler.x), D3DXToDegree(euler.y), D3DXToDegree(euler.z));
    }

    void Util::CopyBackBuffer(UINT width, UINT height, LPDIRECT3DDEVICE9 source, LPDIRECT3DDEVICE9 target, 
        LPDIRECT3DTEXTURE9 *texture)
    {
        if (source == nullptr || target == nullptr)
            return;

        // Create texture with source device and apply the backbuffer.
        LPDIRECT3DSURFACE9 surface;
        source->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);

        // Create a texture with the target device and copy the rendered surface to its context.
        if (FAILED(target->CreateTexture(width, height, 0, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, texture, 0)))
        {
            surface->Release();
            return;
        }

        PDIRECT3DSURFACE9 newTextureSurface;
        (*texture)->GetSurfaceLevel(0, &newTextureSurface);

        // Not the most effcient but works for now. :/
        D3DXLoadSurfaceFromSurface(newTextureSurface, 0, 0, surface, 0, 0, D3DX_DEFAULT, 0);

        newTextureSurface->Release();
        surface->Release();
    }

    float Util::Snap(float value)
    {
        return floorf(value + 0.5f);
    }

    float Util::GetDPIScale()
    {
        HDC dc = GetDC(0);
        const int dpi = GetDeviceCaps(dc, LOGPIXELSX);
        ReleaseDC(0, dc);

        return dpi / 96.0f;
    }

    bool Util::IntersectTriangle(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &v0,
        const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2, float *dist)
    {
        // Find vectors for two edges sharing vert0
        D3DXVECTOR3 edge1 = v1 - v0;
        D3DXVECTOR3 edge2 = v2 - v0;

        // Begin calculating determinant - also used to calculate U parameter.
        D3DXVECTOR3 pvec;
        D3DXVec3Cross(&pvec, &dir, &edge2);

        // If determinant is near zero, ray lies in plane of triangle.
        float det = D3DXVec3Dot(&edge1, &pvec);

        if (det < 0.0001f) return false;

        // Calculate U parameter and test bounds.
        D3DXVECTOR3 tvec = orig - v0;
        float u = D3DXVec3Dot(&tvec, &pvec);
        if (u < 0.0f || u > det) return false;

        // Prepare to test V parameter.
        D3DXVECTOR3 qvec;
        D3DXVec3Cross(&qvec, &tvec, &edge1);

        // Calculate V parameter and test bounds.
        float v = D3DXVec3Dot(&dir, &qvec);
        if (v < 0.0f || u + v > det) return false;

        *dist = D3DXVec3Dot(&edge2, &qvec) * (1.0f / det);

        return true;
    }

    void Util::ScreenRaycast(LPDIRECT3DDEVICE9 device, const D3DXVECTOR2 &screenPoint, const D3DXMATRIX &view,
        D3DXVECTOR3 *origin, D3DXVECTOR3 *dir)
    {
        D3DVIEWPORT9 viewport;
        device->GetViewport(&viewport);

        D3DXMATRIX matProj;
        device->GetTransform(D3DTS_PROJECTION, &matProj);

        D3DXMATRIX matWorld;
        D3DXMatrixIdentity(&matWorld);

        D3DXVECTOR3 v1;
        D3DXVECTOR3 start = D3DXVECTOR3(screenPoint.x, screenPoint.y, 0.0f);
        D3DXVec3Unproject(&v1, &start, &viewport, &matProj, &view, &matWorld);

        D3DXVECTOR3 v2;
        D3DXVECTOR3 end = D3DXVECTOR3(screenPoint.x, screenPoint.y, 1.0f);
        D3DXVec3Unproject(&v2, &end, &viewport, &matProj, &view, &matWorld);

        *origin = v1;
        D3DXVec3Normalize(dir, &(v2 - v1));
    }

    void Util::ProjectToScreenSpace(LPDIRECT3DDEVICE9 device, const D3DXVECTOR3 &worldPoint, const D3DXMATRIX &view, 
        D3DXVECTOR3 *screenPointOut)
    {
        D3DVIEWPORT9 viewport;
        device->GetViewport(&viewport);

        D3DXMATRIX matProj;
        device->GetTransform(D3DTS_PROJECTION, &matProj);

        D3DXMATRIX matWorld;
        D3DXMatrixIdentity(&matWorld);

        D3DXVECTOR3 v0sp;
        D3DXVec3Project(screenPointOut, &worldPoint, &viewport, &matProj, &view, &matWorld);
    }
}
