#pragma once

#include "d3d7_include.h"
#include "ddraw_wrapped_object.h"

namespace dxvk {

  class D3D7VertexBuffer final : public DDrawWrappedObject<d3d9::IDirect3DVertexBuffer9, IDirect3DVertexBuffer7> {

  public:

    D3D7VertexBuffer(IDirect3DVertexBuffer7* buffProxy, Com<d3d9::IDirect3DVertexBuffer9>&& pBuffer, DWORD size, DWORD fvf);

    HRESULT STDMETHODCALLTYPE GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC lpVBDesc);

    HRESULT STDMETHODCALLTYPE Lock(DWORD flags, void **data, DWORD *data_size);

    HRESULT STDMETHODCALLTYPE Unlock();

    HRESULT STDMETHODCALLTYPE ProcessVertices(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPDIRECT3DVERTEXBUFFER7 lpSrcBuffer, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags);

    HRESULT STDMETHODCALLTYPE ProcessVerticesStrided(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags);

    HRESULT STDMETHODCALLTYPE Optimize(LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags);

    DWORD GetFVF() const {
      return m_fvf;
    }

    DWORD GetStride() const {
      return m_stride;
    }

  private:
    DWORD m_size   = 0;
    DWORD m_fvf    = 0;
    DWORD m_stride = 0;

  };

}
