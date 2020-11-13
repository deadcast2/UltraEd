#ifndef _RENDER_DEVICE_H_
#define _RENDER_DEVICE_H_

#include <d3d9.h>
#include <boost/uuid/uuid.hpp>

namespace UltraEd
{
    class RenderDevice
    {
    public:
        RenderDevice(HWND hWnd);
        RenderDevice(UINT width, UINT height);
        ~RenderDevice();
        const LPDIRECT3DDEVICE9 GetDevice();
        const D3DPRESENT_PARAMETERS *GetParameters();
        bool IsLost();
        void Resize(UINT width, UINT height);

    private:
        bool SetupWindow(UINT width, UINT height);
        bool SetupRenderer();

    private:
        boost::uuids::uuid m_id;
        LPDIRECT3DDEVICE9 m_device;
        LPDIRECT3D9 m_d3d9;
        D3DPRESENT_PARAMETERS m_d3dpp;
        WNDCLASSEX m_wc;
        HWND m_hWnd;
    };
}

#endif
