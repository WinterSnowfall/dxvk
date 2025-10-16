#include "d3d7_device.h"
#include "d3d7_interface.h"
#include "d3d7_buffer.h"
#include "d3d7_state_block.h"
#include "ddraw_surface.h"
#include "ddraw_util.h"

#include <cstring>

namespace dxvk {

  D3D7Device::D3D7Device(
      IDirect3DDevice7* d3d7DeviceProxy,
      D3D7Interface* pParent,
      Com<d3d9::IDirect3DDevice9>&& pDevice,
      DDrawSurface* pSurface)
    : DDrawWrappedObject<d3d9::IDirect3DDevice9, IDirect3DDevice7>(std::move(pDevice), d3d7DeviceProxy)
    , m_parent( pParent )
    , m_rt7dev ( pSurface ) {
    // Get the bridge interface to D3D9.
    if (unlikely(FAILED(m_d3d9->QueryInterface(__uuidof(IDxvkD3D8Bridge), reinterpret_cast<void**>(&m_bridge))))) {
      throw DxvkError("D3D7Device: ERROR! Failed to get D3D9 Bridge. d3d9.dll might not be DXVK!");
    }

    m_d3d9->GetRenderTarget(0, &m_rt9);
    m_d3d9->GetDepthStencilSurface(&m_ds9);

    // Initial RT
    m_rt = pSurface;

    // Textures
    m_textures.fill(nullptr);

    // Implicit IB (512kb is more than enough for D3D7)
    m_ibSize = 512 * 1024;
    Logger::info(str::format("D3D7Device: Creating implicit index buffer, size: ", m_ibSize));
    m_d3d9->CreateIndexBuffer(m_ibSize, D3DUSAGE_DYNAMIC, d3d9::D3DFMT_INDEX16, d3d9::D3DPOOL_DEFAULT, &m_ib9, nullptr);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetCaps(D3DDEVICEDESC7 *desc) {
    Logger::debug(">>> D3D7Device::GetCaps");
    *desc = m_parent->GetDesc();
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::EnumTextureFormats(LPD3DENUMPIXELFORMATSCALLBACK cb, void *ctx) {
    Logger::debug("<<< D3D7Device::EnumTextureFormats: Proxy");
    return m_proxy->EnumTextureFormats(cb, ctx);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::BeginScene() {
    Logger::debug(">>> D3D7Device::BeginScene");
    return m_d3d9->BeginScene();
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::EndScene() {
    Logger::debug(">>> D3D7Device::EndScene");
    HRESULT hr = m_d3d9->EndScene();
    //Implicitly present after EndScene
    if (SUCCEEDED(hr))
      m_d3d9->Present(NULL, NULL, NULL, NULL);
    return hr;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetDirect3D(IDirect3D7 **d3d) {
    Logger::debug(">>> D3D7Device::GetDirect3D");
    *d3d = ref(m_parent);
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::SetRenderTarget(IDirectDrawSurface7 *surface, DWORD flags) {
    Logger::debug(">>> D3D7Device::SetRenderTarget");

    DDrawSurface* surface7 = static_cast<DDrawSurface*>(surface);

    if (surface7 == nullptr) {
      Logger::err("D3D7Device::SetRenderTarget: NULL render target");
      return D3DERR_INVALIDCALL;
    }

    // Reset to initial rendertarget.
    if (surface7 == m_rt7dev.ptr()) {
      m_d3d9->SetRenderTarget(0, m_rt9.ptr());
      m_d3d9->SetDepthStencilSurface(m_ds9.ptr());
      m_rt = m_rt7dev.ptr();
      return D3D_OK;
    }

    //We may not have a valid d3d9 objects at this point
    HRESULT hr = surface7->IntializeD3D9();

    if (FAILED(hr)) {
      Logger::err("D3D7Device::SetRenderTarget: Failed to initialize d3d9 RT");
      return hr;
    }

    m_rt = surface7;

    return m_d3d9->SetRenderTarget(0, m_rt->GetSurface());
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetRenderTarget(IDirectDrawSurface7 **surface) {
    Logger::debug(">>> D3D7Device::GetRenderTarget");

    *surface = m_rt.ref();

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::Clear(DWORD count, D3DRECT *rects, DWORD flags, D3DCOLOR color, D3DVALUE z, DWORD stencil) {
    Logger::debug(">>> D3D7Device::Clear");
    return m_d3d9->Clear(count, rects, flags, color, z, stencil);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::SetTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX *matrix) {
    Logger::debug(">>> D3D7Device::SetTransform");
    return m_d3d9->SetTransform(ConvertTransformState(state), matrix);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX *matrix) {
    Logger::debug(">>> D3D7Device::GetTransform");
    return m_d3d9->GetTransform(ConvertTransformState(state), matrix);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::MultiplyTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX *matrix) {
    Logger::debug(">>> D3D7Device::MultiplyTransform");
    return m_d3d9->MultiplyTransform(ConvertTransformState(state), matrix);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::SetViewport(D3DVIEWPORT7 *data) {
    Logger::debug(">>> D3D7Device::SetViewport");
    return m_d3d9->SetViewport(reinterpret_cast<d3d9::D3DVIEWPORT9*>(data));
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetViewport(D3DVIEWPORT7 *data) {
    Logger::debug(">>> D3D7Device::GetViewport");
    return m_d3d9->GetViewport(reinterpret_cast<d3d9::D3DVIEWPORT9*>(data));
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::SetMaterial(D3DMATERIAL7 *data) {
    Logger::debug(">>> D3D7Device::SetMaterial");
    return m_d3d9->SetMaterial(reinterpret_cast<d3d9::D3DMATERIAL9*>(data));
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetMaterial(D3DMATERIAL7 *data) {
    Logger::debug(">>> D3D7Device::GetMaterial");
    return m_d3d9->GetMaterial(reinterpret_cast<d3d9::D3DMATERIAL9*>(data));
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::SetLight(DWORD idx, D3DLIGHT7 *data) {
    Logger::debug(">>> D3D7Device::SetLight");
    return m_d3d9->SetLight(idx, reinterpret_cast<d3d9::D3DLIGHT9*>(data));
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetLight(DWORD idx, D3DLIGHT7 *data) {
    Logger::debug(">>> D3D7Device::GetLight");
    return m_d3d9->GetLight(idx, reinterpret_cast<d3d9::D3DLIGHT9*>(data));
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::SetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState) {
    Logger::debug(">>> D3D7Device::SetRenderState");
    //TODO: A LOT of RS handling
    return m_d3d9->SetRenderState(static_cast<d3d9::D3DRENDERSTATETYPE>(dwRenderStateType), dwRenderState);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetRenderState(D3DRENDERSTATETYPE dwRenderStateType, LPDWORD lpdwRenderState) {
    Logger::debug(">>> D3D7Device::GetRenderState");
    //TODO: A LOT of reverse RS handling
    return m_d3d9->GetRenderState(static_cast<d3d9::D3DRENDERSTATETYPE>(dwRenderStateType), lpdwRenderState);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::BeginStateBlock() {
    Logger::debug(">>> D3D7Device::BeginStateBlock");

    if (unlikely(m_recorder != nullptr))
      return D3DERR_INVALIDCALL;

    HRESULT hr = m_d3d9->BeginStateBlock();

    if (likely(SUCCEEDED(hr))) {
      m_handle++;
      auto stateBlockIterPair = m_stateBlocks.emplace(std::piecewise_construct,
                                                      std::forward_as_tuple(m_handle),
                                                      std::forward_as_tuple(this));
      m_recorder = &stateBlockIterPair.first->second;
      m_recorderHandle = m_handle;
    }

    return hr;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::EndStateBlock(LPDWORD lpdwBlockHandle) {
    Logger::debug(">>> D3D7Device::EndStateBlock");

    if (unlikely(lpdwBlockHandle == nullptr || m_recorder == nullptr))
      return D3DERR_INVALIDCALL;

    Com<d3d9::IDirect3DStateBlock9> pStateBlock;
    HRESULT hr = m_d3d9->EndStateBlock(&pStateBlock);

    if (likely(SUCCEEDED(hr))) {
      m_recorder->SetD3D9(std::move(pStateBlock));

      *lpdwBlockHandle = m_recorderHandle;

      m_recorder = nullptr;
      m_recorderHandle = 0;
    }

    return hr;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::ApplyStateBlock(DWORD dwBlockHandle) {
    Logger::debug(">>> D3D7Device::ApplyStateBlock");

    // Applications cannot apply a state block while another is being recorded
    if (unlikely(ShouldRecord()))
      return D3DERR_INVALIDCALL;

    auto stateBlockIter = m_stateBlocks.find(dwBlockHandle);

    if (unlikely(stateBlockIter == m_stateBlocks.end())) {
      Logger::warn(str::format("D3D7Device::ApplyStateBlock: Invalid dwBlockHandle: ", std::hex, dwBlockHandle));
      return D3D_OK;
    }

    return stateBlockIter->second.Apply();
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::CaptureStateBlock(DWORD dwBlockHandle) {
    Logger::debug(">>> D3D7Device::CaptureStateBlock");

    // Applications cannot capture a state block while another is being recorded
    if (unlikely(ShouldRecord()))
      return D3DERR_INVALIDCALL;

    auto stateBlockIter = m_stateBlocks.find(dwBlockHandle);

    if (unlikely(stateBlockIter == m_stateBlocks.end())) {
      Logger::warn(str::format("D3D7Device::CaptureStateBlock: Invalid dwBlockHandle: ", std::hex, dwBlockHandle));
      return D3D_OK;
    }

    return stateBlockIter->second.Capture();
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::DeleteStateBlock(DWORD dwBlockHandle) {
    Logger::debug(">>> D3D7Device::DeleteStateBlock");

    // Applications cannot delete a state block while another is being recorded
    if (unlikely(ShouldRecord()))
      return D3DERR_INVALIDCALL;

    auto stateBlockIter = m_stateBlocks.find(dwBlockHandle);

    if (unlikely(stateBlockIter == m_stateBlocks.end())) {
      Logger::warn(str::format("D3D7Device::DeleteStateBlock: Invalid dwBlockHandle: ", std::hex, dwBlockHandle));
      return D3D_OK;
    }

    m_stateBlocks.erase(stateBlockIter);

    // native apparently does drop the handle counter in
    // situations where the handle being removed is the
    // last allocated handle, which allows some reuse
    if (m_handle == dwBlockHandle)
      m_handle--;

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::CreateStateBlock(D3DSTATEBLOCKTYPE d3dsbType, LPDWORD lpdwBlockHandle) {
    Logger::debug(">>> D3D7Device::CreateStateBlock");

    if (unlikely(lpdwBlockHandle == nullptr))
      return D3DERR_INVALIDCALL;

    // Applications cannot create a state block while another is being recorded
    if (unlikely(ShouldRecord()))
      return D3DERR_INVALIDCALL;

    D3D7StateBlockType stateBlockType = ConvertStateBlockType(d3dsbType);

    if (unlikely(stateBlockType == D3D7StateBlockType::Unknown)) {
      Logger::warn(str::format("D3D7Device::CreateStateBlock: Invalid state block type: ", d3dsbType));
      return D3DERR_INVALIDCALL;
    }

    Com<d3d9::IDirect3DStateBlock9> pStateBlock9;
    HRESULT res = m_d3d9->CreateStateBlock(d3d9::D3DSTATEBLOCKTYPE(d3dsbType), &pStateBlock9);

    if (likely(SUCCEEDED(res))) {
      m_handle++;
      m_stateBlocks.emplace(std::piecewise_construct,
                            std::forward_as_tuple(m_handle),
                            std::forward_as_tuple(this, stateBlockType, pStateBlock9.ptr()));
      *lpdwBlockHandle = m_handle;
    }

    return res;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::PreLoad(IDirectDrawSurface7 *surface) {
    Logger::debug(">>> D3D7Device::PreLoad");
    DDrawSurface* surface7 = reinterpret_cast<DDrawSurface*>(surface);

    if (surface7 != nullptr) {
      // Doesn't return anything in D3D9
      surface7->GetSurface()->PreLoad();
    }

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::DrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags) {
    Logger::debug(">>> D3D7Device::DrawPrimitive");

    GetD3D9()->SetFVF(dwVertexTypeDesc);
    return GetD3D9()->DrawPrimitiveUP(
                          d3d9::D3DPRIMITIVETYPE(dptPrimitiveType),
                          dwVertexCount / GetPrimitiveSize(dptPrimitiveType),
                          lpvVertices,
                          GetFVFSize(dwVertexTypeDesc));
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::DrawIndexedPrimitive(D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags) {
    Logger::debug(">>> D3D7Device::DrawIndexedPrimitive");

    m_d3d9->SetFVF(dwVertexTypeDesc);
    return m_d3d9->DrawIndexedPrimitiveUP(
                          d3d9::D3DPRIMITIVETYPE(d3dptPrimitiveType),
                          0,
                          dwVertexCount,
                          dwIndexCount / GetPrimitiveSize(d3dptPrimitiveType),
                          lpwIndices,
                          d3d9::D3DFMT_INDEX16,
                          lpvVertices,
                          GetFVFSize(dwVertexTypeDesc));
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::SetClipStatus(D3DCLIPSTATUS *clip_status) {
    Logger::debug(">>> D3D7Device::SetClipStatus");

    d3d9::D3DCLIPSTATUS9 clipStatus9;
    //TODO: Split the union and intersection flags
    clipStatus9.ClipUnion        = clip_status->dwStatus;
    clipStatus9.ClipIntersection = clip_status->dwStatus;

    return m_d3d9->SetClipStatus(&clipStatus9);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetClipStatus(D3DCLIPSTATUS *clip_status) {
    Logger::debug(">>> D3D7Device::GetClipStatus");

    d3d9::D3DCLIPSTATUS9 clipStatus9;
    HRESULT hr = m_d3d9->GetClipStatus(&clipStatus9);

    if (FAILED(hr))
      return hr;

    D3DCLIPSTATUS clipStatus7 = { };
    clipStatus7.dwFlags  = D3DCLIPSTATUS_STATUS;
    clipStatus7.dwStatus = D3DSTATUS_DEFAULT | clipStatus9.ClipUnion | clipStatus9.ClipIntersection;

    *clip_status = clipStatus7;

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::DrawPrimitiveStrided(D3DPRIMITIVETYPE primitive_type, DWORD fvf, D3DDRAWPRIMITIVESTRIDEDDATA *pStridedData, DWORD stridedDataCount, DWORD flags) {
    Logger::warn("!!! D3D7Device::DrawPrimitiveStrided: Stub");
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::DrawIndexedPrimitiveStrided(D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD  dwVertexTypeDesc, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD  dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags) {
    Logger::warn("!!! D3D7Device::DrawIndexedPrimitiveStrided: Stub");
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::DrawPrimitiveVB(D3DPRIMITIVETYPE d3dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, DWORD dwFlags) {
    Logger::debug(">>> D3D7Device::DrawPrimitiveVB");

    D3D7VertexBuffer* vb = static_cast<D3D7VertexBuffer*>(lpd3dVertexBuffer);
    m_d3d9->SetFVF(vb->GetFVF());
    m_d3d9->SetStreamSource(0, vb->GetD3D9(), 0, vb->GetStride());
    return m_d3d9->DrawPrimitive(
                       d3d9::D3DPRIMITIVETYPE(d3dptPrimitiveType),
                       dwStartVertex,
                       dwNumVertices / GetPrimitiveSize(d3dptPrimitiveType));
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE d3dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags) {
    Logger::debug(">>> D3D7Device::DrawIndexedPrimitiveVB");

    D3D7VertexBuffer* vb = static_cast<D3D7VertexBuffer*>(lpd3dVertexBuffer);
    UploadIndices(lpwIndices, dwIndexCount);
    m_d3d9->SetFVF(vb->GetFVF());
    m_d3d9->SetStreamSource(0, vb->GetD3D9(), 0, vb->GetStride());
    m_d3d9->SetIndices(m_ib9.ptr());
    return m_d3d9->DrawIndexedPrimitive(
                        d3d9::D3DPRIMITIVETYPE(d3dptPrimitiveType),
                        0,
                        0,
                        dwNumVertices,
                        dwStartVertex,
                        dwIndexCount / GetPrimitiveSize(d3dptPrimitiveType));
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::ComputeSphereVisibility(D3DVECTOR *centers, D3DVALUE *radii, DWORD sphere_count, DWORD sphereCount, DWORD *visibility) {
    Logger::warn("!!! D3D7Device::ComputeSphereVisibility: Stub");
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetTexture(DWORD stage, IDirectDrawSurface7 **surface) {
    Logger::debug(">>> D3D7Device::GetTexture");

    *surface = m_textures[stage].ref();

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::SetTexture(DWORD stage, IDirectDrawSurface7 *surface) {
    Logger::debug(">>> D3D7Device::SetTexture");

    if (unlikely(stage >= caps::MaxTextureStages))
      return D3DERR_INVALIDCALL;

    if (unlikely(ShouldRecord()))
      return m_recorder->SetTexture(stage, surface);

    DDrawSurface* surface7 = static_cast<DDrawSurface*>(surface);

    if (surface7 != nullptr) {
      //We may not have a valid d3d9 objects at this point
      HRESULT hr = surface7->IntializeD3D9();

      if (FAILED(hr)) {
        Logger::err("D3D7Device::SetTexture: Failed to initialize d3d9 texture");
        return hr;
      }
    }

    if (m_textures[stage].ptr() == surface7)
      return D3D_OK;

    m_textures[stage] == surface7;

    if (m_textures[stage] != nullptr) {
      return m_d3d9->SetTexture(stage, m_textures[stage].ptr()->GetTexture());
    } else {
      return m_d3d9->SetTexture(stage, nullptr);
    }
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE d3dTexStageStateType, LPDWORD lpdwState) {
    Logger::debug(">>> D3D7Device::GetTextureStageState");

    d3d9::D3DSAMPLERSTATETYPE stateType = ConvertSamplerStateType(d3dTexStageStateType);

    if (stateType != -1u) {
      // if the type has been remapped to a sampler state type:
      return GetD3D9()->GetSamplerState(dwStage, stateType, lpdwState);
    }
    else {
      return GetD3D9()->GetTextureStageState(dwStage, d3d9::D3DTEXTURESTAGESTATETYPE(d3dTexStageStateType), lpdwState);
    }
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE d3dTexStageStateType, DWORD dwState) {
    Logger::debug(">>> D3D7Device::SetTextureStageState");

    d3d9::D3DSAMPLERSTATETYPE stateType = ConvertSamplerStateType(d3dTexStageStateType);

    if (stateType != -1) {
      // if the type has been remapped to a sampler state type:
      return m_d3d9->SetSamplerState(dwStage, stateType, dwState);
    } else {
      return m_d3d9->SetTextureStageState(dwStage, d3d9::D3DTEXTURESTAGESTATETYPE(d3dTexStageStateType), dwState);
    }
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::ValidateDevice(LPDWORD lpdwPasses) {
    Logger::warn("!!! D3D7Device::ValidateDevice: Stub");
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::Load(IDirectDrawSurface7 *dst_surface, POINT *dst_point, IDirectDrawSurface7 *src_surface, RECT *src_rect, DWORD flags) {
    Logger::warn("!!! D3D7Device::Load: Stub");
    //TODO: Should this actually do anything in D3D9?
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::LightEnable(DWORD dwLightIndex, WINBOOL bEnable) {
    Logger::debug(">>> D3D7Device::LightEnable");
    return m_d3d9->LightEnable(dwLightIndex, bEnable);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetLightEnable(DWORD dwLightIndex, WINBOOL *pbEnable) {
    Logger::debug(">>> D3D7Device::GetLightEnable");
    return m_d3d9->GetLightEnable(dwLightIndex, pbEnable);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::SetClipPlane(DWORD dwIndex, D3DVALUE *pPlaneEquation) {
    Logger::debug(">>> D3D7Device::SetClipPlane");
    return m_d3d9->SetClipPlane(dwIndex, pPlaneEquation);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetClipPlane(DWORD dwIndex, D3DVALUE *pPlaneEquation) {
    Logger::debug(">>> D3D7Device::GetClipPlane");
    return m_d3d9->GetClipPlane(dwIndex, pPlaneEquation);
  }

  HRESULT STDMETHODCALLTYPE D3D7Device::GetInfo(DWORD info_id, void *info, DWORD info_size) {
    Logger::debug("<<< D3D7Device::GetInfo: Proxy");
    //TODO: D3D9 queries?
    return m_proxy->GetInfo(info_id, info, info_size);
  }

  void D3D7Device::UploadIndices(void* indices, DWORD indexCount) {
    size_t size = indexCount * sizeof(WORD);

    static size_t maxSize = std::max(size, maxSize);

    Logger::debug(str::format("D3D7Device::UploadIndices: Uploading ", size, " indices, max ", maxSize));

    if (size > m_ibSize) {
      Logger::err("D3D7Device::UploadIndices: Implicit IB capacity exceeded");
    }

    void* pData = nullptr;
    //TODO: OOM with D3DLOCK_DISCARD and direct mapping
    m_ib9->Lock(0, size, &pData, 0);
    memcpy(pData, indices, size);
    m_ib9->Unlock();
  }

}