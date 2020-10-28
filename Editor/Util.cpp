#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include "Util.h"

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

    void Util::CopyBackBuffer(UINT width, UINT height, LPDIRECT3DDEVICE9 source, LPDIRECT3DDEVICE9 target, 
        LPDIRECT3DTEXTURE9 *texture)
    {
        if (source == nullptr || target == nullptr)
            return;

        // Create texture with source device and apply the backbuffer.
        LPDIRECT3DSURFACE9 surface;
        source->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);

        // Create a texture with the target device and copy the rendered surface to its context.
        if (FAILED(target->CreateTexture(width, height, 0, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, texture, 0)))
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
}
