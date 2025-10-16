#include "d3d7_interface.h"
#include "d3d7_device.h"
#include "d3d7_buffer.h"
#include "ddraw_surface.h"
#include "ddraw_util.h"

#include <cstring>

namespace dxvk {

  //TODO: For some reason these aren't imported properly from the headers
  const GUID IID_IDirect3DTnLHalDevice = { 0xf5049e78, 0x4861, 0x11d2, {0xa4, 0x07, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0xa8} };
  const GUID IID_IDirect3DHALDevice    = { 0x84E63dE0, 0x46AA, 0x11CF, {0x81, 0x6F, 0x00, 0x00, 0xC0, 0x20, 0x15, 0x6E} };

  D3D7Interface::D3D7Interface(IDirect3D7* d3d7IntfProxy)
    : DDrawWrappedObject<d3d9::IDirect3D9, IDirect3D7>(std::move(d3d9::Direct3DCreate9(D3D_SDK_VERSION)), d3d7IntfProxy) {
    // Get the bridge interface to D3D9.
    if (unlikely(FAILED(m_d3d9->QueryInterface(__uuidof(IDxvkD3D8InterfaceBridge), reinterpret_cast<void**>(&m_bridge))))) {
      throw DxvkError("D3D7Interface: ERROR! Failed to get D3D9 Bridge. d3d9.dll might not be DXVK!");
    }

    //TODO:
    //m_bridge->EnableD3D7CompatibilityMode();
  }

  HRESULT STDMETHODCALLTYPE D3D7Interface::EnumDevices(LPD3DENUMDEVICESCALLBACK7 cb, void *ctx) {
    Logger::debug(">>> D3D7Interface::EnumDevices");

    UINT count = m_d3d9->GetAdapterCount();

    for (UINT i = 0; i < count; i++) {
      d3d9::D3DCAPS9 caps;
      m_d3d9->GetDeviceCaps(i, d3d9::D3DDEVTYPE_HAL, &caps);

      D3DPRIMCAPS prim;
      prim.dwSize = sizeof(D3DPRIMCAPS);
      prim.dwMiscCaps           |= 0x7F;
      prim.dwRasterCaps         |= 0xFFFFFFFF;
      prim.dwZCmpCaps           |= 0xFFFFFFFF;
      prim.dwSrcBlendCaps       |= 0xFFFFFFFF;
      prim.dwDestBlendCaps      |= 0xFFFFFFFF;
      prim.dwAlphaCmpCaps       |= 0xFFFFFFFF;
      prim.dwShadeCaps          |= 0xFFFFFFFF;
      prim.dwTextureCaps        |= 0xFFFFFFFF;
      prim.dwTextureFilterCaps  |= 0xFFFFFFFF;
      prim.dwTextureBlendCaps   |= 0xFFFFFFFF;
      prim.dwTextureAddressCaps |= 0xFFFFFFFF;
      prim.dwStippleWidth        = 32;
      prim.dwStippleHeight       = 32;

      m_desc.dwDevCaps = D3DDEVCAPS_FLOATTLVERTEX
                     | D3DDEVCAPS_EXECUTESYSTEMMEMORY
                     | D3DDEVCAPS_EXECUTEVIDEOMEMORY
                     | D3DDEVCAPS_TLVERTEXSYSTEMMEMORY
                     | D3DDEVCAPS_TLVERTEXVIDEOMEMORY
                     | D3DDEVCAPS_TEXTURESYSTEMMEMORY
                     | D3DDEVCAPS_TEXTUREVIDEOMEMORY
                     | D3DDEVCAPS_DRAWPRIMTLVERTEX
                     | D3DDEVCAPS_CANRENDERAFTERFLIP
                     | D3DDEVCAPS_TEXTURENONLOCALVIDMEM
                     | D3DDEVCAPS_DRAWPRIMITIVES2
                     | D3DDEVCAPS_SEPARATETEXTUREMEMORIES
                     | D3DDEVCAPS_DRAWPRIMITIVES2EX
                     | D3DDEVCAPS_HWTRANSFORMANDLIGHT
                     | D3DDEVCAPS_CANBLTSYSTONONLOCAL
                     | D3DDEVCAPS_HWRASTERIZATION;
      m_desc.dpcLineCaps = prim;
      m_desc.dpcTriCaps  = prim;
      m_desc.dwDeviceRenderBitDepth   = DDBD_16 | DDBD_24 | DDBD_32;
      m_desc.dwDeviceZBufferBitDepth  = DDBD_16 | DDBD_24 | DDBD_32;
      m_desc.dwMinTextureWidth        = 0;
      m_desc.dwMinTextureHeight       = 0;
      m_desc.dwMaxTextureWidth        = caps.MaxTextureWidth;
      m_desc.dwMaxTextureHeight       = caps.MaxTextureHeight;
      m_desc.dwMaxTextureRepeat       = caps.MaxTextureRepeat;
      m_desc.dwMaxTextureAspectRatio  = caps.MaxTextureAspectRatio;
      m_desc.dwMaxAnisotropy          = caps.MaxAnisotropy;
      m_desc.dvGuardBandLeft          = caps.GuardBandLeft;
      m_desc.dvGuardBandTop           = caps.GuardBandTop;
      m_desc.dvGuardBandRight         = caps.GuardBandRight;
      m_desc.dvGuardBandBottom        = caps.GuardBandBottom;
      m_desc.dvExtentsAdjust          = caps.ExtentsAdjust;
      m_desc.dwStencilCaps            = caps.StencilCaps;
      m_desc.dwFVFCaps                = caps.FVFCaps;
      m_desc.dwTextureOpCaps          = caps.TextureOpCaps;
      m_desc.wMaxTextureBlendStages   = (WORD)caps.MaxTextureBlendStages;
      m_desc.wMaxSimultaneousTextures = (WORD)caps.MaxSimultaneousTextures;
      m_desc.dwMaxActiveLights        = caps.MaxActiveLights;
      m_desc.dvMaxVertexW             = caps.MaxVertexW;
      m_desc.wMaxUserClipPlanes       = (WORD)caps.MaxUserClipPlanes;
      m_desc.wMaxVertexBlendMatrices  = (WORD)caps.MaxVertexBlendMatrices;
      m_desc.dwVertexProcessingCaps   = caps.VertexProcessingCaps;

      //TODO: Check what these are supposed to be on native d3d7
      m_desc.deviceGUID = IID_IDirect3DHALDevice;
      char deviceNameHAL[100] = "HAL Adapter";
      char deviceDescHAL[100] = "Direct3D HAL";

      //TODO: Will anything be overwritten during the callback?
      D3DDEVICEDESC7 descCallback = m_desc;

      cb(&deviceNameHAL[0], &deviceDescHAL[0], &descCallback, ctx);

      m_desc.deviceGUID = IID_IDirect3DTnLHalDevice;
      char deviceNameTNL[100] = "T&L Adapter";
      char deviceDescTNL[100] = "Direct3D with T&L Support";

      cb(&deviceNameTNL[0], &deviceDescTNL[0], &descCallback, ctx);
    }

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Interface::CreateDevice(const IID& rclsid, IDirectDrawSurface7 *surface, IDirect3DDevice7 **ppd3dDevice) {
    Logger::debug(">>> D3D7Interface::CreateDevice");

    DDrawSurface* ddraw7Surface = static_cast<DDrawSurface*>(surface);

    if (ddraw7Surface == nullptr) {
      Logger::err("D3D7Interface::CreateDevice: Null surface provided");
      return D3DERR_INVALIDCALL;
    }

    DDSURFACEDESC2 desc;
    desc.dwSize = sizeof(DDSURFACEDESC2);
    ddraw7Surface->GetSurfaceDesc(&desc);

    Logger::info(str::format("D3D7Interface::CreateDevice: RenderTarget: ", desc.dwWidth, "x", desc.dwHeight));

    if (m_hwnd == nullptr) {
      Logger::err("D3D7Interface::CreateDevice: HWND is NULL");
      return D3DERR_NOTAVAILABLE;
    }

    d3d9::D3DPRESENT_PARAMETERS params;
    params.BackBufferWidth    = desc.dwWidth;
    params.BackBufferHeight   = desc.dwHeight;
    params.BackBufferFormat   = d3d9::D3DFMT_UNKNOWN;
    params.BackBufferCount    = 1;
    params.MultiSampleType    = d3d9::D3DMULTISAMPLE_NONE;
    params.MultiSampleQuality = 0;
    params.SwapEffect         = d3d9::D3DSWAPEFFECT_DISCARD;
    params.hDeviceWindow      = m_hwnd;
    params.Windowed           = TRUE;
    params.EnableAutoDepthStencil     = TRUE;
    params.AutoDepthStencilFormat     = d3d9::D3DFMT_D16;
    params.Flags                      = 0;
    params.FullScreen_RefreshRateInHz = 0;
    params.PresentationInterval       = D3DPRESENT_INTERVAL_ONE;

    Com<d3d9::IDirect3DDevice9> device = nullptr;

    HRESULT hr = m_d3d9->CreateDevice(
      D3DADAPTER_DEFAULT,
      d3d9::D3DDEVTYPE_HAL,
      m_hwnd,
      D3DCREATE_HARDWARE_VERTEXPROCESSING,
      &params,
      &device
    );
    if (FAILED(hr)) {
      Logger::err("D3D7Interface::CreateDevice: Failed to created the nested D3D9 device");
      return hr;
    }

    IDirect3DDevice7* d3d7DeviceProxy = nullptr;
    hr = m_proxy->CreateDevice(rclsid, ddraw7Surface->GetProxy(), &d3d7DeviceProxy);
    if (FAILED(hr)) {
      Logger::err("D3D7Interface::CreateDevice: Failed to created the proxy device");
      return hr;
    }

    try{
      m_device = new D3D7Device(d3d7DeviceProxy, this, std::move(device), ddraw7Surface);
    } catch (const DxvkError& e) {
      Logger::err(e.message());
      *ppd3dDevice = nullptr;
      return D3DERR_NOTAVAILABLE;
    }

    *ppd3dDevice = m_device.ref();

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Interface::CreateVertexBuffer(D3DVERTEXBUFFERDESC *desc, IDirect3DVertexBuffer7 **ppVertexBuffer, DWORD usage) {
    Logger::debug(">>> D3D7Interface::CreateVertexBuffer");

    IDirect3DVertexBuffer7* vertexBuffer7 = nullptr;
    HRESULT hr = m_proxy->CreateVertexBuffer(desc, &vertexBuffer7, usage);
    if (FAILED(hr)) {
      Logger::err("D3D7Interface::CreateVertexBuffer: Failed to create proxy vertex buffer");
      return hr;
    }

    // Can't create anything without a valid device
    if (m_device == nullptr)
      return D3DERR_INVALIDCALL;

    Com<d3d9::IDirect3DVertexBuffer9> buffer = nullptr;
    DWORD Stride = GetFVFSize(desc->dwFVF);
    DWORD Size   = Stride * desc->dwNumVertices;
    hr = m_device->GetD3D9()->CreateVertexBuffer(Size, D3DUSAGE_DYNAMIC, desc->dwFVF, d3d9::D3DPOOL_DEFAULT, &buffer, nullptr);

    *ppVertexBuffer = ref(new D3D7VertexBuffer(vertexBuffer7, std::move(buffer), Size, desc->dwFVF));

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D7Interface::EnumZBufferFormats(const IID& device_iid, LPD3DENUMPIXELFORMATSCALLBACK cb, void *ctx) {
    Logger::debug("<<< D3D7Interface::EnumZBufferFormats: Proxy");
    return m_proxy->EnumZBufferFormats(device_iid, cb, ctx);
  }

  HRESULT STDMETHODCALLTYPE D3D7Interface::EvictManagedTextures() {
    Logger::debug(">>> D3D7Interface::EvictManagedTextures");
    if (m_device != nullptr)
      m_device->GetD3D9()->EvictManagedResources();
    return m_proxy->EvictManagedTextures();
  }

}