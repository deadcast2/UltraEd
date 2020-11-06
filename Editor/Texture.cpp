#include "Project.h"
#include "Texture.h"

namespace UltraEd
{
    Texture::Texture() :
        m_texture(0),
        m_textureId()
    { }

    Texture::~Texture()
    {
        Delete();
    }

    bool Texture::Load(IDirect3DDevice9 *device, const boost::uuids::uuid &textureId)
    {
        Delete();

        const auto assetPath = Project::GetAssetPath(textureId);
        if (!assetPath.empty())
        {
            m_textureId = textureId;

            return SUCCEEDED(D3DXCreateTextureFromFileEx(device, assetPath.string().c_str(),
                D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT,
                D3DX_DEFAULT, 0, 0, 0, &m_texture));
        }

        return false;
    }

    LPDIRECT3DTEXTURE9 Texture::Get()
    {
        return m_texture;
    }

    const boost::uuids::uuid &Texture::GetId()
    {
        return m_textureId;
    }

    std::array<int, 2> Texture::Dimensions()
    {
        D3DSURFACE_DESC desc;
        if (m_texture != NULL && SUCCEEDED(m_texture->GetLevelDesc(0, &desc)))
        {
            int width = static_cast<int>(desc.Width);
            int height = static_cast<int>(desc.Height);
            return { width, height };
        }
        return { 0, 0 };
    }

    bool Texture::IsValid(std::string &reason)
    {
        const std::vector<int> validSizes { 4, 8, 16, 32, 64 };
        const auto dimensions = Dimensions();
        const auto isXValid = std::find(validSizes.cbegin(), validSizes.cend(), dimensions[0]) != validSizes.cend();
        const auto isYValid = std::find(validSizes.cbegin(), validSizes.cend(), dimensions[1]) != validSizes.cend();

        if (!isXValid || !isYValid)
        {
            reason = std::string("Invalid dimensions");
            return false;
        }

        return true;
    }

    void Texture::Delete()
    {
        if (IsLoaded())
        {
            m_texture->Release();
            m_texture = 0;
            m_textureId = boost::uuids::nil_uuid();
        }
    }

    bool Texture::IsLoaded()
    {
        return m_texture != nullptr;
    }
}
