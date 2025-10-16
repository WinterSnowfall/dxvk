#include <algorithm>

#include "ddraw_surface.h"
#include "ddraw_interface.h"
#include "ddraw_util.h"
#include "d3d7_caps.h"
#include "d3d7_device.h"

namespace dxvk {

  DDrawSurface::DDrawSurface(IDirectDrawSurface7* surfProxy, DD7Interface* pParent, DDSURFACEDESC2 desc)
    : DDrawWrappedObject<d3d9::IDirect3DSurface9, IDirectDrawSurface7>(nullptr, surfProxy)
    , m_parent ( pParent )
    , m_desc ( desc ) {}

  HRESULT STDMETHODCALLTYPE DDrawSurface::AddAttachedSurface(LPDIRECTDRAWSURFACE7 lpDDSAttachedSurface) {
    Logger::debug(">>> DDrawSurface::AddAttachedSurface");

    DDrawSurface* ddraw7Surface = static_cast<DDrawSurface*>(lpDDSAttachedSurface);

    if (ddraw7Surface != nullptr) {
      auto it = std::find(m_attachedSurfaces.begin(), m_attachedSurfaces.end(), ddraw7Surface);
      // already attached
      if (it != m_attachedSurfaces.end()) {
          Logger::debug("DDrawSurface::AddAttachedSurface: Pre-existing wrapped surface found");
          return DD_OK;
      }

      return m_proxy->AddAttachedSurface(ddraw7Surface->GetProxy());
    }

    return DD_OK;
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::AddOverlayDirtyRect(LPRECT lpRect) {
    Logger::debug("<<< DDrawSurface::AddOverlayDirtyRect: Proxy");
    return m_proxy->AddOverlayDirtyRect(lpRect);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx) {
    Logger::debug("<<< DDrawSurface::Blt: Proxy");

    if (IsFrontBuffer())
      return DD_OK;

    DDrawSurface* ddraw7Surface = static_cast<DDrawSurface*>(lpDDSrcSurface);

    return m_proxy->Blt(lpDestRect, ddraw7Surface->GetProxy(), lpSrcRect, dwFlags, lpDDBltFx);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::BltBatch(LPDDBLTBATCH lpDDBltBatch, DWORD dwCount, DWORD dwFlags) {
    Logger::debug("<<< DDrawSurface::BltBatch: Proxy");
    return m_proxy->BltBatch(lpDDBltBatch, dwCount, dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans) {
    Logger::debug("<<< DDrawSurface::BltFast: Proxy");

    DDrawSurface* ddraw7Surface = static_cast<DDrawSurface*>(lpDDSrcSurface);

    return m_proxy->BltFast(dwX, dwY, ddraw7Surface->GetProxy(), lpSrcRect, dwTrans);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::DeleteAttachedSurface(DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSAttachedSurface) {
    Logger::debug(">>> DDrawSurface::DeleteAttachedSurface");

    DDrawSurface* ddraw7Surface = static_cast<DDrawSurface*>(lpDDSAttachedSurface);

    if (ddraw7Surface != nullptr) {
      auto it = std::find(m_attachedSurfaces.begin(), m_attachedSurfaces.end(), ddraw7Surface);
      if (it != m_attachedSurfaces.end()) {
          Logger::debug("DDrawSurface::DeleteAttachedSurface: Pre-existing wrapped surface found");
          m_attachedSurfaces.erase(it);
      }

      return m_proxy->DeleteAttachedSurface(dwFlags, ddraw7Surface->GetProxy());
    }

    return DD_OK;
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::EnumAttachedSurfaces(LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback) {
    Logger::debug("<<< DDrawSurface::EnumAttachedSurfaces: Proxy");
    return m_proxy->EnumAttachedSurfaces(lpContext, lpEnumSurfacesCallback);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::EnumOverlayZOrders(DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpfnCallback) {
    Logger::debug("<<< DDrawSurface::EnumOverlayZOrders: Proxy");
    return m_proxy->EnumOverlayZOrders(dwFlags, lpContext, lpfnCallback);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::Flip(LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags) {
    Logger::debug("--- DDrawSurface::Flip: Ignored");

    if (lpDDSurfaceTargetOverride != nullptr)
      Logger::warn("DDrawSurface::Flip: use of non-NULL lpDDSurfaceTargetOverride");

    //return m_proxy->Flip(lpDDSurfaceTargetOverride, dwFlags);
    return DD_OK;
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetAttachedSurface(LPDDSCAPS2 lpDDSCaps, LPDIRECTDRAWSURFACE7 *lplpDDAttachedSurface) {
    Logger::debug(">>> DDrawSurface::GetAttachedSurface");

    LPDIRECTDRAWSURFACE7 surface;
    HRESULT hr = m_proxy->GetAttachedSurface(lpDDSCaps, &surface);

    if (FAILED(hr)) {
      Logger::err("DDrawSurface::GetAttachedSurface: Failed to find the requested surface");
      return hr;
    }

    *lplpDDAttachedSurface = nullptr;
    DDrawSurface* ddraw7Surface = static_cast<DDrawSurface*>(surface);

    if (ddraw7Surface != nullptr) {
      auto it = std::find(m_attachedSurfaces.begin(), m_attachedSurfaces.end(), ddraw7Surface);
      if (it != m_attachedSurfaces.end()) {
        Logger::debug("DDrawSurface::GetAttachedSurface: Pre-existing wrapped surface found");
        *lplpDDAttachedSurface = ddraw7Surface->GetProxy();
        return DD_OK;
      }

      DDSURFACEDESC2 desc;
      surface->GetSurfaceDesc(&desc);
      Com<DDrawSurface> returnSurface = new DDrawSurface(surface, m_parent, desc);
      m_attachedSurfaces.push_back(returnSurface.ptr());
      *lplpDDAttachedSurface = returnSurface.ref();
    } else {
      Logger::err("DDrawSurface::GetAttachedSurface: Got a null proxy surface");
    }

    return DD_OK;
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetBltStatus(DWORD dwFlags) {
    Logger::debug("<<< DDrawSurface::GetBltStatus: Proxy");
    return m_proxy->GetBltStatus(dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetCaps(LPDDSCAPS2 lpDDSCaps) {
    Logger::debug("<<< DDrawSurface::GetCaps: Proxy");
    return m_proxy->GetCaps(lpDDSCaps);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetClipper(IDirectDrawClipper **lplpDDClipper) {
    Logger::debug("<<< DDrawSurface::GetClipper: Proxy");
    return m_proxy->GetClipper(lplpDDClipper);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) {
    Logger::debug("<<< DDrawSurface::GetColorKey: Proxy");
    return m_proxy->GetColorKey(dwFlags, lpDDColorKey);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetDC(HDC *lphDC) {
    Logger::debug("<<< DDrawSurface::GetDC: Proxy");
    return m_proxy->GetDC(lphDC);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetFlipStatus(DWORD dwFlags) {
    Logger::debug("<<< DDrawSurface::GetFlipStatus: Proxy");
    return m_proxy->GetFlipStatus(dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetOverlayPosition(LPLONG lplX, LPLONG lplY) {
    Logger::debug("<<< DDrawSurface::GetOverlayPosition: Proxy");
    return m_proxy->GetOverlayPosition(lplX, lplY);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetPalette(IDirectDrawPalette **lplpDDPalette) {
    Logger::debug("<<< DDrawSurface::GetPalette: Proxy");
    return m_proxy->GetPalette(lplpDDPalette);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat) {
    Logger::debug("<<< DDrawSurface::GetPixelFormat: Proxy");
    return m_proxy->GetPixelFormat(lpDDPixelFormat);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC2 lpDDSurfaceDesc) {
    Logger::debug("<<< DDrawSurface::GetSurfaceDesc: Proxy");
    return m_proxy->GetSurfaceDesc(lpDDSurfaceDesc);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::Initialize(LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc) {
    Logger::debug("<<< DDrawSurface::Initialize: Proxy");
    return m_proxy->Initialize(lpDD, lpDDSurfaceDesc);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::IsLost() {
    Logger::debug("<<< DDrawSurface::IsLost: Proxy");
    return m_proxy->IsLost();
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::Lock(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent) {
    Logger::debug("<<< DDrawSurface::Lock: Proxy");
    return m_proxy->Lock(lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::ReleaseDC(HDC hDC) {
    Logger::debug("<<< DDrawSurface::ReleaseDC: Proxy");
    return m_proxy->ReleaseDC(hDC);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::Restore() {
    Logger::debug("<<< DDrawSurface::Restore: Proxy");
    return m_proxy->Restore();
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper) {
    Logger::debug("<<< DDrawSurface::SetClipper: Proxy");
    return m_proxy->SetClipper(lpDDClipper);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) {
    Logger::debug("<<< DDrawSurface::SetColorKey: Proxy");
    return m_proxy->SetColorKey(dwFlags, lpDDColorKey);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::SetOverlayPosition(LONG lX, LONG lY) {
    Logger::debug("<<< DDrawSurface::SetOverlayPosition: Proxy");
    return m_proxy->SetOverlayPosition(lX, lY);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::SetPalette(LPDIRECTDRAWPALETTE lpDDPalette) {
    Logger::debug("<<< DDrawSurface::SetPalette: Proxy");
    return m_proxy->SetPalette(lpDDPalette);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::Unlock(LPRECT lpSurfaceData) {
    Logger::debug("<<< DDrawSurface::Unlock: Proxy");

    HRESULT hr = m_proxy->Unlock(lpSurfaceData);

    //TODO: Optimize this a bit, instead of uploading after each copy
    if (SUCCEEDED(hr))
      UploadTextureData();

    return hr;
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::UpdateOverlay(LPRECT lpSrcRect, LPDIRECTDRAWSURFACE7 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx) {
    Logger::debug("<<< DDrawSurface::UpdateOverlay: Proxy");

    DDrawSurface* ddraw7Surface = static_cast<DDrawSurface*>(lpDDDestSurface);

    return m_proxy->UpdateOverlay(lpSrcRect, ddraw7Surface->GetProxy(), lpDestRect, dwFlags, lpDDOverlayFx);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::UpdateOverlayDisplay(DWORD dwFlags) {
    Logger::debug("<<< DDrawSurface::UpdateOverlayDisplay: Proxy");
    return m_proxy->UpdateOverlayDisplay(dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::UpdateOverlayZOrder(DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSReference) {
    Logger::debug("<<< DDrawSurface::UpdateOverlayZOrder: Proxy");

    DDrawSurface* ddraw7Surface = static_cast<DDrawSurface*>(lpDDSReference);

    return m_proxy->UpdateOverlayZOrder(dwFlags, ddraw7Surface->GetProxy());
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetDDInterface(void **lplpDD) {
    Logger::debug("<<< DDrawSurface::GetDDInterface: Proxy");
    return m_proxy->GetDDInterface(lplpDD);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::PageLock(DWORD dwFlags) {
    Logger::debug("<<< DDrawSurface::PageLock: Proxy");
    return m_proxy->PageLock(dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::PageUnlock(DWORD dwFlags) {
    Logger::debug("<<< DDrawSurface::PageUnlock: Proxy");
    return m_proxy->PageUnlock(dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::SetSurfaceDesc(LPDDSURFACEDESC2 lpDDSD, DWORD dwFlags) {
    Logger::debug("<<< DDrawSurface::SetSurfaceDesc: Proxy");
    m_desc = *lpDDSD;
    return m_proxy->SetSurfaceDesc(lpDDSD, dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::SetPrivateData(const GUID &tag, LPVOID pData, DWORD cbSize, DWORD dwFlags) {
    Logger::debug("<<< DDrawSurface::SetPrivateData: Proxy");
    return m_proxy->SetPrivateData(tag, pData, cbSize, dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetPrivateData(const GUID &tag, LPVOID pBuffer, LPDWORD pcbBufferSize) {
    Logger::debug("<<< DDrawSurface::GetPrivateData: Proxy");
    return m_proxy->GetPrivateData(tag, pBuffer, pcbBufferSize);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::FreePrivateData(const GUID &tag) {
    Logger::debug("<<< DDrawSurface::FreePrivateData: Proxy");
    return m_proxy->FreePrivateData(tag);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetUniquenessValue(LPDWORD pValue) {
    Logger::debug("<<< DDrawSurface::GetUniquenessValue: Proxy");
    return m_proxy->GetUniquenessValue(pValue);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::ChangeUniquenessValue() {
    Logger::debug("<<< Called DDrawSurface::ChangeUniquenessValue: Proxy");
    return m_proxy->ChangeUniquenessValue();
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::SetPriority(DWORD prio) {
    Logger::debug("<<< DDrawSurface::SetPriority: Proxy");
    return m_proxy->SetPriority(prio);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetPriority(LPDWORD prio) {
    Logger::debug("<<< DDrawSurface::GetPriority: Proxy");
    return m_proxy->GetPriority(prio);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::SetLOD(DWORD lod) {
    Logger::debug("<<< DDrawSurface::SetLOD: Proxy");
    return m_proxy->SetLOD(lod);
  }

  HRESULT STDMETHODCALLTYPE DDrawSurface::GetLOD(LPDWORD lod) {
    Logger::debug("<<< DDrawSurface::GetLOD: Proxy");
    return m_proxy->GetLOD(lod);
  }

  HRESULT DDrawSurface::IntializeD3D9() {
    // Already initialized;
    if (m_d3d9 != nullptr || m_texture != nullptr)
      return DD_OK;

    refreshD3D7Device();

    if (m_d3d7device == nullptr) {
      Logger::warn("DDrawSurface::IntializeD3D9: Null D3D7 device, can't initalize right now");
      return DD_OK;
    }

    Logger::info("DDrawSurface::IntializeD3D9: Initializing D3D9 objects...");

    HRESULT hr;

    //TODO: This sometimes ends up with ridiculous amounts (maybe underruns?)
    uint32_t mips = std::min(static_cast<uint32_t>(m_desc.dwMipMapCount + 1), caps::MaxMipLevels);

    //TODO: CubeMaps
    if (m_desc.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP) {
      Logger::warn("DDrawSurface::IntializeD3D9: Skipping cube map initalization");
      return DD_OK;
    }

    /*Com<d3d9::IDirect3DCubeTexture9> tex = nullptr;

      // TODO: Pool for non-RT cubemaps.
      hr = GetD3D9()->CreateCubeTexture(
        desc.dwWidth, mips, renderTarget ? D3DUSAGE_RENDERTARGET : 0,
        ConvertFormat(desc.ddpfPixelFormat), d3d9::D3DPOOL_DEFAULT, &tex, nullptr);

      if (FAILED(hr)) return hr;

      if (!renderTarget) {
        // TODO: Upload cubemap faces.
      }

      // Attach face 0 to this surface.
      Com<d3d9::IDirect3DSurface9> face = nullptr;
      tex->GetCubeMapSurface((d3d9::D3DCUBEMAP_FACES)0, 0, &face);
      surface->SetSurface(std::move(face));

      // Attach sides 1-5 to each attached surface.
      surface->EnumAttachedSurfaces(tex.ptr(),
        [](IDirectDrawSurface7* subsurf, DDSURFACEDESC2* desc, void* ctx) WINAPI -> HRESULT {
          d3d9::IDirect3DCubeTexture9* cube = (d3d9::IDirect3DCubeTexture9*)ctx;

          // Skip zbuffer.
          if (desc->ddsCaps.dwCaps & DDSCAPS_ZBUFFER)
            return DDENUMRET_OK;

          Com<d3d9::IDirect3DSurface9> face = nullptr;
          cube->GetCubeMapSurface(GetCubemapFace(desc), 0, &face);

          DD7Surface* dd7surf = (DD7Surface*)subsurf;
          dd7surf->SetSurface(std::move(face));

          return DDENUMRET_OK;
        }
      );

      surface->SetTexture(std::move(tex));
      return D3D_OK;
    }*/

    if (IsRenderTarget()) {
      Logger::debug("A render target, allegedly...");

      Com<d3d9::IDirect3DSurface9> rt = nullptr;

      if (m_desc.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) {
        hr = m_d3d7device->GetD3D9()->GetBackBuffer(0, 0, d3d9::D3DBACKBUFFER_TYPE_MONO, &rt);
        Logger::info("DDrawSurface::IntializeD3D9: Created front buffer surface.");
      }

      else if (m_desc.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER) {
        hr = m_d3d7device->GetD3D9()->GetBackBuffer(0, 0, d3d9::D3DBACKBUFFER_TYPE_MONO, &rt);
        Logger::info("DDrawSurface::IntializeD3D9: Created back buffer surface.");
      }

      else if (m_desc.ddsCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN) {
        hr = m_d3d7device->GetD3D9()->CreateOffscreenPlainSurface(
          m_desc.dwWidth, m_desc.dwHeight, ConvertFormat(m_desc.ddpfPixelFormat),
          d3d9::D3DPOOL_DEFAULT, &rt, nullptr);
        Logger::info("DDrawSurface::IntializeD3D9: Created offscreen plain surface.");
      }

      else if (m_desc.ddsCaps.dwCaps & DDSCAPS_COMPLEX) {

        hr = m_d3d7device->GetD3D9()->CreateRenderTarget(
          m_desc.dwWidth, m_desc.dwHeight, ConvertFormat(m_desc.ddpfPixelFormat),
          d3d9::D3DMULTISAMPLE_NONE, 0, FALSE, &rt, nullptr);

        Logger::warn("DDrawSurface::IntializeD3D9: Unknown complex surface RT. Creating generic RT.");
      }

      else {
        hr = m_d3d7device->GetD3D9()->CreateRenderTarget(
          m_desc.dwWidth, m_desc.dwHeight, ConvertFormat(m_desc.ddpfPixelFormat),
          d3d9::D3DMULTISAMPLE_NONE, 0, FALSE, &rt, nullptr);
        Logger::info("DDrawSurface::IntializeD3D9: Created generic RT.");
      }

      if (FAILED(hr)) {
        Logger::err("DDrawSurface::IntializeD3D9: Failed to create RT");
        return hr;
      }

      m_d3d9 = std::move(rt);

    } else {
      Logger::debug("Not a render target, also allegedly...");

      Com<d3d9::IDirect3DTexture9> tex = nullptr;

      //TODO: Always MANAGED?
      hr = m_d3d7device->GetD3D9()->CreateTexture(
        m_desc.dwWidth, m_desc.dwHeight, mips, 0,
        ConvertFormat(m_desc.ddpfPixelFormat), d3d9::D3DPOOL_MANAGED, &tex, nullptr);

      if (FAILED(hr)) {
        Logger::err("DDrawSurface::IntializeD3D9: Failed to create texture");
        m_texture = nullptr;
        return hr;
      }

      // Get ddraw surface DC
      HDC dc7;
      hr = this->GetDC(&dc7);
      if (FAILED(hr)) {
        Logger::err("DDrawSurface::IntializeD3D9: Failed GetDC for main surface");
        return hr;
      }

      // Blit to each mip level
      for (uint32_t i = 0; i < mips; i++) {
        Com<d3d9::IDirect3DSurface9> level = nullptr;
        hr = tex->GetSurfaceLevel(i, &level);
        if (FAILED(hr)) {
          Logger::warn(str::format("DDrawSurface::IntializeD3D9: Failed to get surface for level ", i));
          continue;
        }

        HDC levelDC;
        hr = level->GetDC(&levelDC);
        if (FAILED(hr)) {
          Logger::warn(str::format("DDrawSurface::IntializeD3D9: Failed GetDC for mip level ", i));
          continue;
        }

        BitBlt(levelDC, 0, 0, m_desc.dwWidth, m_desc.dwHeight, dc7, 0, 0, SRCCOPY);

        level->ReleaseDC(levelDC);
      }

      this->ReleaseDC(dc7);
      m_texture = std::move(tex);
    }

    return DD_OK;
  }

  inline HRESULT DDrawSurface::UploadTextureData() {
    IntializeD3D9();

    // Nothing to upload
    if (m_d3d9 == nullptr && m_texture == nullptr)
      return DD_OK;

    HRESULT hr;

    Logger::info("DDrawSurface::UploadTextureData: Blitting to D3D9...");

    // Get ddraw surface DC
    HDC dc7;
    hr = this->GetDC(&dc7);
    if (FAILED(hr)) {
      Logger::err("DDrawSurface::UploadTextureData: Failed GetDC for main surface");
      return hr;
    }

    Com<d3d9::IDirect3DSurface9> level = nullptr;

    // Blit all the mips for textures
    if (m_texture != nullptr) {
      //TODO: This sometimes ends up with ridiculous amounts (maybe underruns?)
      uint32_t mips = std::min(static_cast<uint32_t>(m_desc.dwMipMapCount + 1), caps::MaxMipLevels);

      for (uint32_t i = 0; i < mips; i++) {
        hr = m_texture->GetSurfaceLevel(i, &level);
        if (FAILED(hr)) {
          Logger::warn(str::format("DDrawSurface::UploadTextureData: Failed to get surface for level ", i));
          continue;
        }

        HDC levelDC;
        hr = level->GetDC(&levelDC);
        if (FAILED(hr)) {
          Logger::warn(str::format("DDrawSurface::UploadTextureData: Failed GetDC for mip level ", i));
          continue;
        }

        BitBlt(levelDC, 0, 0, m_desc.dwWidth, m_desc.dwHeight, dc7, 0, 0, SRCCOPY);

        level->ReleaseDC(levelDC);
      }
    // Blit surfaces directly
    } else if (m_d3d9 != nullptr) {
      level = m_d3d9.ptr();

      HDC levelDC;
      hr = level->GetDC(&levelDC);
      if (FAILED(hr)) {
        Logger::err("DDrawSurface::UploadTextureData: Failed GetDC for surface");
        return hr;
      }

      BitBlt(levelDC, 0, 0, m_desc.dwWidth, m_desc.dwHeight, dc7, 0, 0, SRCCOPY);

      level->ReleaseDC(levelDC);
    } else {
      // Shouldn't ever happen, we've checked for it above
      Logger::err("DDrawSurface::UploadTextureData: DDrawSurface has neither a d3d9 surface or texture attached");
    }

    this->ReleaseDC(dc7);

    return DD_OK;
  }

}
