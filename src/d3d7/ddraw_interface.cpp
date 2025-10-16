#include "ddraw_interface.h"
#include "d3d7_interface.h"
#include "ddraw_surface.h"

#include <cstring>

namespace dxvk {

  DD7Interface::DD7Interface(LPVOID lplpDD)
    : m_proxy ( static_cast<IDirectDraw7*>(lplpDD) ) {
      void* d3d7IntfProxy = nullptr;
      m_proxy->QueryInterface(__uuidof(IDirect3D7), &d3d7IntfProxy);
      m_d3d7Intf = new D3D7Interface(static_cast<IDirect3D7*>(d3d7IntfProxy));
    }

  HRESULT STDMETHODCALLTYPE DD7Interface::QueryInterface(REFIID riid, void** ppvObject) {
    Logger::debug(">>> DD7Interface::QueryInterface");

    if (ppvObject == nullptr)
      return E_POINTER;

    *ppvObject = nullptr;

    if (riid == __uuidof(IUnknown)
     || riid == __uuidof(IDirectDraw7)) {
      *ppvObject = ref(this);
      return S_OK;
    }

    if (riid == __uuidof(IDirect3D7)) {
      *ppvObject = m_d3d7Intf.ref();
      return S_OK;
    }

    Logger::warn("DD7Interface::QueryInterface: Unknown interface query");
    Logger::warn(str::format(riid));
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::Compact() {
    Logger::debug("<<< DD7Interface::Compact: Proxy");
    return m_proxy->Compact();
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, IUnknown *pUnkOuter) {
    Logger::debug("<<< DD7Interface::CreateClipper: Proxy");
    return m_proxy->CreateClipper(dwFlags, lplpDDClipper, pUnkOuter);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpColorTable, LPDIRECTDRAWPALETTE *lplpDDPalette, IUnknown *pUnkOuter) {
    Logger::debug("<<< DD7Interface::CreatePalette: Proxy");
    return m_proxy->CreatePalette(dwFlags, lpColorTable, lplpDDPalette, pUnkOuter);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::CreateSurface(LPDDSURFACEDESC2 lpDDSurfaceDesc, LPDIRECTDRAWSURFACE7 *lplpDDSurface, IUnknown *pUnkOuter) {
    Logger::debug(">>> DD7Interface::CreateSurface");

    LPDIRECTDRAWSURFACE7 lplpDDSurfaceProxy = nullptr;
    HRESULT hr = m_proxy->CreateSurface(lpDDSurfaceDesc, &lplpDDSurfaceProxy, pUnkOuter);

    if (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_BACKBUFFER)
      Logger::info("DD7Interface::CreateSurface: Created a back buffer surface");

    if (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
      Logger::info("DD7Interface::CreateSurface: Created a front buffer surface");

    if (SUCCEEDED(hr) && lplpDDSurfaceProxy != nullptr) {
      *lplpDDSurface = ref(new DDrawSurface(static_cast<IDirectDrawSurface7*>(lplpDDSurfaceProxy), this, *lpDDSurfaceDesc));
    } else {
      Logger::err("DD7Interface::CreateSurface: Failed to create proxy surface");
    }

    return hr;
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::DuplicateSurface(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDIRECTDRAWSURFACE7 *lplpDupDDSurface) {
    Logger::debug("<<< DD7Interface::DuplicateSurface: Proxy");

    DDrawSurface* ddraw7Surface = static_cast<DDrawSurface*>(lpDDSurface);

    //TODO: Wrap the duplicate before returning
    return m_proxy->DuplicateSurface(ddraw7Surface->GetProxy(), lplpDupDDSurface);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK2 lpEnumModesCallback) {
    Logger::debug("<<< DD7Interface::EnumDisplayModes: Proxy");
    return m_proxy->EnumDisplayModes(dwFlags, lpDDSurfaceDesc, lpContext, lpEnumModesCallback);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::EnumSurfaces(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSD, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback) {
    Logger::debug("<<< DD7Interface::EnumSurfaces: Proxy");
    return m_proxy->EnumSurfaces(dwFlags, lpDDSD, lpContext, lpEnumSurfacesCallback);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::FlipToGDISurface() {
    Logger::debug("<<< DD7Interface::FlipToGDISurface: Proxy");
    return m_proxy->FlipToGDISurface();
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::GetCaps(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps) {
    Logger::debug("<<< DD7Interface::GetCaps: Proxy");
    return m_proxy->GetCaps(lpDDDriverCaps, lpDDHELCaps);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::GetDisplayMode(LPDDSURFACEDESC2 lpDDSurfaceDesc) {
    Logger::debug("<<< DD7Interface::GetDisplayMode: Proxy");
    return m_proxy->GetDisplayMode(lpDDSurfaceDesc);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::GetFourCCCodes(LPDWORD lpNumCodes, LPDWORD lpCodes) {
    Logger::debug("<<< DD7Interface::GetFourCCCodes: Proxy");
    return m_proxy->GetFourCCCodes(lpNumCodes, lpCodes);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::GetGDISurface(IDirectDrawSurface7** lplpGDIDDSurface) {
    Logger::debug("<<< DD7Interface::GetGDISurface: Proxy");
    return m_proxy->GetGDISurface(lplpGDIDDSurface);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::GetMonitorFrequency(LPDWORD lpdwFrequency) {
    Logger::debug("<<< DD7Interface::GetMonitorFrequency: Proxy");
    return m_proxy->GetMonitorFrequency(lpdwFrequency);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::GetScanLine(LPDWORD lpdwScanLine) {
    Logger::debug("<<< DD7Interface::GetScanLine: Proxy");
    return m_proxy->GetScanLine(lpdwScanLine);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::GetVerticalBlankStatus(WINBOOL* lpbIsInVB) {
    Logger::debug("<<< DD7Interface::GetVerticalBlankStatus: Proxy");
    return m_proxy->GetVerticalBlankStatus(lpbIsInVB);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::Initialize(GUID* lpGUID) {
    Logger::debug("<<< DD7Interface::Initialize: Proxy");
    return m_proxy->Initialize(lpGUID);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::RestoreDisplayMode() {
    Logger::debug("<<< DD7Interface::RestoreDisplayMode: Proxy");
    return m_proxy->RestoreDisplayMode();
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::SetCooperativeLevel(HWND hWnd, DWORD dwFlags) {
    Logger::debug("<<< DD7Interface::SetCooperativeLevel: Proxy");
    m_d3d7Intf->SetHwnd(hWnd);
    return m_proxy->SetCooperativeLevel(hWnd, dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags) {
    Logger::debug("<<< DD7Interface::SetDisplayMode: Proxy");
    return m_proxy->SetDisplayMode(dwWidth, dwHeight, dwBPP, dwRefreshRate, dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::WaitForVerticalBlank(DWORD dwFlags, HANDLE hEvent) {
    Logger::debug("<<< DD7Interface::WaitForVerticalBlank: Proxy");
    return m_proxy->WaitForVerticalBlank(dwFlags, hEvent);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::GetAvailableVidMem(LPDDSCAPS2 lpDDCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree) {
    Logger::debug("<<< DD7Interface::GetAvailableVidMem: Proxy");
    return m_proxy->GetAvailableVidMem(lpDDCaps, lpdwTotal, lpdwFree);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::GetSurfaceFromDC(HDC hdc, IDirectDrawSurface7** pSurf) {
    Logger::debug(">>> DD7Interface::GetSurfaceFromDC");

    IDirectDrawSurface7* surface = nullptr;
    HRESULT hr = m_proxy->GetSurfaceFromDC(hdc, &surface);

    if (FAILED(hr)) {
      Logger::err("DD7Interface::GetSurfaceFromDC: Failed to get surface from DC");
      return hr;
    }

    DDSURFACEDESC2 desc;
    surface->GetSurfaceDesc(&desc);
    *pSurf = ref(new DDrawSurface(surface, this, desc));

    return hr;
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::RestoreAllSurfaces() {
    Logger::debug("<<< DD7Interface::RestoreAllSurfaces: Proxy");
    return m_proxy->RestoreAllSurfaces();
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::TestCooperativeLevel() {
    Logger::debug("<<< DD7Interface::TestCooperativeLevel: Proxy");
    return m_proxy->TestCooperativeLevel();
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2 pDDDI, DWORD dwFlags) {
    Logger::debug("<<< DD7Interface::GetDeviceIdentifier: Proxy");
    return m_proxy->GetDeviceIdentifier(pDDDI, dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::StartModeTest(LPSIZE pModes, DWORD dwNumModes, DWORD dwFlags) {
    Logger::debug("<<< DD7Interface::StartModeTest: Proxy");
    return m_proxy->StartModeTest(pModes, dwNumModes, dwFlags);
  }

  HRESULT STDMETHODCALLTYPE DD7Interface::EvaluateMode(DWORD dwFlags, DWORD* pTimeout) {
    Logger::debug("<<< DD7Interface::EvaluateMode: Proxy");
    return m_proxy->EvaluateMode(dwFlags, pTimeout);
  }

}