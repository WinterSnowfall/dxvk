#include "d3d7_buffer.h"
#include "d3d7_device.h"
#include "ddraw_util.h"

namespace dxvk {

  D3D7VertexBuffer::D3D7VertexBuffer(IDirect3DVertexBuffer7* buffProxy, Com<d3d9::IDirect3DVertexBuffer9>&& pBuffer, DWORD size, DWORD fvf)
    : DDrawWrappedObject<d3d9::IDirect3DVertexBuffer9, IDirect3DVertexBuffer7>(std::move(pBuffer), buffProxy)
    , m_size(size)
    , m_fvf(fvf)
    , m_stride(GetFVFSize(fvf)) {}

    HRESULT STDMETHODCALLTYPE D3D7VertexBuffer::GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC lpVBDesc) {
        Logger::debug(">>> D3D7VertexBuffer::GetVertexBufferDesc");

        d3d9::D3DVERTEXBUFFER_DESC bufferDesc;
        m_d3d9->GetDesc(&bufferDesc);

        lpVBDesc->dwSize         = sizeof(D3DVERTEXBUFFERDESC);
        //TODO: Other caps
        lpVBDesc->dwCaps         = bufferDesc.Pool == d3d9::D3DPOOL_SYSTEMMEM ? D3DVBCAPS_SYSTEMMEMORY : 0;
        lpVBDesc->dwFVF          = m_fvf;
        lpVBDesc->dwNumVertices  = m_size / m_stride;

        return D3D_OK;
    }

    HRESULT STDMETHODCALLTYPE D3D7VertexBuffer::Lock(DWORD flags, void **data, DWORD *data_size) {
        Logger::debug(">>> D3D7VertexBuffer::Lock");

        if (data_size != nullptr)
            *data_size = m_size;

        if (!(flags & DDLOCK_WAIT))
            flags |= D3DLOCK_DONOTWAIT;

        return m_d3d9->Lock(0, 0, data, flags);
    }

    HRESULT STDMETHODCALLTYPE D3D7VertexBuffer::Unlock() {
        Logger::debug(">>> D3D7VertexBuffer::Unlock");
        return m_d3d9->Unlock();
    }

    HRESULT STDMETHODCALLTYPE D3D7VertexBuffer::ProcessVertices(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPDIRECT3DVERTEXBUFFER7 lpSrcBuffer, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags) {
        Logger::debug(">>> D3D7VertexBuffer::ProcessVertices");

        D3D7Device* device = reinterpret_cast<D3D7Device*>(lpD3DDevice);
        D3D7VertexBuffer* vb = reinterpret_cast<D3D7VertexBuffer*>(lpSrcBuffer);

        device->GetD3D9()->SetStreamSource(0, vb->GetD3D9(), 0, m_stride);
        return device->GetD3D9()->ProcessVertices(dwSrcIndex, dwDestIndex, dwCount, m_d3d9.ptr(), nullptr, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE D3D7VertexBuffer::ProcessVerticesStrided(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags) {
        Logger::warn("!!! D3D7VertexBuffer::ProcessVerticesStrided: Stub");
        return D3D_OK;
    }

    HRESULT STDMETHODCALLTYPE D3D7VertexBuffer::Optimize(LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags) {
        Logger::warn("!!! D3D7VertexBuffer::Optimize: Stub");
        return D3D_OK;
    };
}
