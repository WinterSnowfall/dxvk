#pragma once

#include "d3d7_include.h"
#include "ddraw_wrapped_object.h"
#include "../d3d9/d3d9_bridge.h"

namespace dxvk {

  class D3D7Device;

  /**
  * \brief D3D7 interface implementation
  *
  * Implements the IDirect3D7 interfaces
  */
  class D3D7Interface final : public DDrawWrappedObject<d3d9::IDirect3D9, IDirect3D7> {

  public:
    D3D7Interface(IDirect3D7* d3d7Intf);

    HRESULT STDMETHODCALLTYPE EnumDevices(LPD3DENUMDEVICESCALLBACK7 cb, void *ctx);

    HRESULT STDMETHODCALLTYPE CreateDevice(const IID& rclsid, IDirectDrawSurface7 *surface, IDirect3DDevice7 **ppd3dDevice);

    HRESULT STDMETHODCALLTYPE CreateVertexBuffer(D3DVERTEXBUFFERDESC *desc, IDirect3DVertexBuffer7 **ppVertexBuffer, DWORD usage);

    HRESULT STDMETHODCALLTYPE EnumZBufferFormats(const IID& device_iid, LPD3DENUMPIXELFORMATSCALLBACK cb, void *ctx);

    HRESULT STDMETHODCALLTYPE EvictManagedTextures();

    void SetHwnd(HWND hwnd) {
      if (m_hwnd == nullptr)
        m_hwnd = hwnd;
    }

    D3D7Device* GetDevice() const {
      return m_device.ptr();
    }

    const D3DDEVICEDESC7& GetDesc() const {
      return m_desc;
    }

  private:
    Com<IDirect3D7>                                 m_d3d7IntfProxy;

    Com<IDxvkD3D8InterfaceBridge>                   m_bridge;

    D3DDEVICEDESC7                                  m_desc;

    Com<D3D7Device, false>                          m_device;
    HWND                                            m_hwnd = nullptr;

  };

}