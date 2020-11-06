#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <d3dx9.h>

namespace UltraEd
{
    class Texture
    {
    public:
        Texture();
        ~Texture();
        bool Load(IDirect3DDevice9 *device, const boost::uuids::uuid &textureId);
        LPDIRECT3DTEXTURE9 Get();
        const boost::uuids::uuid &GetId();
        bool IsLoaded();
        bool IsValid(std::string &reason);
        void Delete();
        std::array<int, 2> Dimensions();

    private:
        LPDIRECT3DTEXTURE9 m_texture;
        boost::uuids::uuid m_textureId;
    };
}

#endif
