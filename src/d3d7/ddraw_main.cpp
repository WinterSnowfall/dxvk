#include "ddraw_interface.h"

namespace dxvk {

  Logger Logger::s_instance("d3d7.log");

  HMODULE GetRealDDrawModule() {
    static HMODULE hDDraw = LoadLibraryA("C:\\windows\\system32\\ddraw.dll");
    return hDDraw;
  }

  HRESULT CreateDirectDrawEx(GUID *lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown *pUnkOuter) {
    if (iid != __uuidof(IDirectDraw7))
      return DDERR_INVALIDPARAMS;

    try {
      HMODULE hDDraw = GetRealDDrawModule();

      if (!hDDraw) {
        Logger::err("CreateDirectDrawEx: Failed to load proxy ddraw.dll");
        *lplpDD = nullptr;
        return DDERR_GENERIC;
      }

      typedef HRESULT (__stdcall *DirectDrawCreateEx_t)(GUID *lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown *pUnkOuter);
      DirectDrawCreateEx_t RealDirectDrawCreateEx = reinterpret_cast<DirectDrawCreateEx_t>(GetProcAddress(hDDraw, "DirectDrawCreateEx"));

      if (!RealDirectDrawCreateEx) {
        Logger::err("CreateDirectDrawEx: Failed GetProcAddress");
        *lplpDD = nullptr;
        return DDERR_GENERIC;
      }

      LPVOID lplpDDReal = nullptr;
      HRESULT hr = RealDirectDrawCreateEx(lpGUID, &lplpDDReal, iid, pUnkOuter);

      if (FAILED(hr) || !lplpDDReal) {
        Logger::err("CreateDirectDrawEx: Failed call to proxy interface");
        *lplpDD = nullptr;
        return hr;
      }

      *lplpDD = ref(new DD7Interface(lplpDDReal));
    } catch (const DxvkError& e) {
      Logger::err(e.message());
      *lplpDD = nullptr;
      return DDERR_GENERIC;
    }

    return DD_OK;
  }

}

extern "C" {

  DLLEXPORT HRESULT __stdcall AcquireDDThreadLock() {
    dxvk::Logger::warn("!!! AcquireDDThreadLock: Stub");
    return DD_OK;
  }

  DLLEXPORT HRESULT __stdcall CompleteCreateSysmemSurface(DWORD arg) {
    dxvk::Logger::warn("!!! CompleteCreateSysmemSurface: Stub");
    return DD_OK;
  }

  DLLEXPORT HRESULT __stdcall D3DParseUnknownCommand(LPVOID lpCmd, LPVOID *lpRetCmd) {
    dxvk::Logger::warn("!!! D3DParseUnknownCommand: Stub");
    return DD_OK;
  }

  DLLEXPORT HRESULT __stdcall DDGetAttachedSurfaceLcl(DWORD arg1, DWORD arg2, DWORD arg3) {
    dxvk::Logger::warn("!!! DDGetAttachedSurfaceLcl: Stub");
    return DD_OK;
  }

  DLLEXPORT HRESULT __stdcall DDInternalLock(DWORD arg1, DWORD arg2) {
    dxvk::Logger::warn("!!! DDInternalLock: Stub");
    return DD_OK;
  }

  DLLEXPORT HRESULT __stdcall DDInternalUnlock(DWORD arg) {
    dxvk::Logger::warn("!!! DDInternalUnlock: Stub");
    return DD_OK;
  }

  DLLEXPORT HRESULT __stdcall DirectDrawCreate(GUID *lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter) {
    dxvk::Logger::debug("<<< DirectDrawCreate: Proxy");

    dxvk::Logger::warn("DirectDrawCreate is a forwarded interface only, expect breakage");

    HMODULE hDDraw = dxvk::GetRealDDrawModule();

    if (!hDDraw) {
      dxvk::Logger::err("DirectDrawCreate: Failed to load proxy ddraw.dll");
      return DDERR_GENERIC;
    }

    typedef HRESULT (__stdcall *DirectDrawCreate_t)(GUID *lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter);
    DirectDrawCreate_t RealDirectDrawCreate = nullptr;
    RealDirectDrawCreate = reinterpret_cast<DirectDrawCreate_t>(GetProcAddress(hDDraw, "DirectDrawCreate"));

    if (!RealDirectDrawCreate) {
      dxvk::Logger::err("DirectDrawCreate: Failed GetProcAddress");
      return DDERR_GENERIC;
    }

    HRESULT hr = RealDirectDrawCreate(lpGUID, lplpDD, pUnkOuter);

    if (FAILED(hr)) {
      dxvk::Logger::err("DirectDrawCreate: Failed call to proxy interface");
    }

    return hr;
  }

  HRESULT WINAPI DirectDrawCreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, IUnknown *pUnkOuter) {
    dxvk::Logger::warn("!!! DirectDrawCreateClipper: Stub");
    return DD_OK;
  }

  DLLEXPORT HRESULT __stdcall DirectDrawCreateEx(GUID *lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown *pUnkOuter) {
    dxvk::Logger::debug(">>> DirectDrawCreateEx");
    return dxvk::CreateDirectDrawEx(lpGUID, lplpDD, iid, pUnkOuter);
  }

  DLLEXPORT HRESULT __stdcall DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext) {
    dxvk::Logger::debug("<<< DirectDrawEnumerateA: Proxy");

    HMODULE hDDraw = dxvk::GetRealDDrawModule();

    if (!hDDraw) {
      dxvk::Logger::err("DirectDrawEnumerateA: Failed to load proxy ddraw.dll");
      return DDERR_GENERIC;
    }

    typedef HRESULT (__stdcall *DirectDrawEnumerateA_t)(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
    DirectDrawEnumerateA_t RealDirectDrawEnumerateA = nullptr;
    RealDirectDrawEnumerateA = reinterpret_cast<DirectDrawEnumerateA_t>(GetProcAddress(hDDraw, "DirectDrawEnumerateA"));

    if (!RealDirectDrawEnumerateA) {
      dxvk::Logger::err("DirectDrawEnumerateA: Failed GetProcAddress");
      return DDERR_GENERIC;
    }

    HRESULT hr = RealDirectDrawEnumerateA(lpCallback, lpContext);

    if (FAILED(hr)) {
      dxvk::Logger::err("DirectDrawEnumerateA: Failed call to proxy interface");
    }

    return hr;
  }

  DLLEXPORT HRESULT __stdcall DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags) {
    dxvk::Logger::debug("<<< DirectDrawEnumerateExA: Proxy");

    HMODULE hDDraw = dxvk::GetRealDDrawModule();

    if (!hDDraw) {
      dxvk::Logger::err("DirectDrawEnumerateExA: Failed to load proxy ddraw.dll");
      return DDERR_GENERIC;
    }

    typedef HRESULT (__stdcall *DirectDrawEnumerateA_t)(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
    DirectDrawEnumerateA_t RealDirectDrawEnumerateExA = nullptr;
    RealDirectDrawEnumerateExA = reinterpret_cast<DirectDrawEnumerateA_t>(GetProcAddress(hDDraw, "DirectDrawEnumerateExA"));

    if (!RealDirectDrawEnumerateExA) {
      dxvk::Logger::err("DirectDrawEnumerateExA: Failed GetProcAddress");
      return DDERR_GENERIC;
    }

    HRESULT hr = RealDirectDrawEnumerateExA(lpCallback, lpContext, dwFlags);

    if (FAILED(hr)) {
      dxvk::Logger::err("DirectDrawEnumerateExA: Failed call to proxy interface");
    }

    return hr;
  }

  DLLEXPORT HRESULT __stdcall DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags) {
    dxvk::Logger::warn("!!! DirectDrawEnumerateExW: Stub");
    return DD_OK;
  }

  DLLEXPORT HRESULT __stdcall DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext) {
    dxvk::Logger::warn("!!! DirectDrawEnumerateW: Stub");
    return DD_OK;
  }

  DLLEXPORT HRESULT __stdcall DllCanUnloadNow() {
    dxvk::Logger::warn("!!! DllCanUnloadNow: Stub");
    return S_OK;
  }

  DLLEXPORT HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv) {
    dxvk::Logger::warn("!!! DllGetClassObject: Stub");
    return S_OK;
  }

  DLLEXPORT HRESULT __stdcall GetDDSurfaceLocal(DWORD arg1, DWORD arg2, DWORD arg3) {
    dxvk::Logger::warn("!!! GetDDSurfaceLocal: Stub");
    return DD_OK;
  }

  DLLEXPORT HRESULT __stdcall GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS, DWORD arg) {
    dxvk::Logger::warn("!!! GetSurfaceFromDC: Stub");
    return S_OK;
  }

  DLLEXPORT HRESULT __stdcall RegisterSpecialCase(DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4) {
    dxvk::Logger::warn("!!! RegisterSpecialCase: Stub");
    return S_OK;
  }

  DLLEXPORT HRESULT __stdcall ReleaseDDThreadLock() {
    dxvk::Logger::warn("!!! ReleaseDDThreadLock: Stub");
    return DD_OK;
  }

}
