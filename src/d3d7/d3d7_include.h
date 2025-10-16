#pragma once

#ifndef _MSC_VER
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0A00
#endif

#include <stdint.h>
#include <d3d.h>

// Declare __uuidof for DDRAW/D3D7 interfaces
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IDirectDraw7,            0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b);
__CRT_UUID_DECL(IDirectDrawSurface7,     0x06675a80,0x3b9b,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b);
__CRT_UUID_DECL(IDirectDrawGammaControl, 0x69c11c3e,0xb46b,0x11d1,0xad,0x7a,0x00,0xc0,0x4f,0xc2,0x9b,0x4e);
__CRT_UUID_DECL(IDirect3D7,              0xf5049e77,0x4861,0x11d2,0xa4,0x07,0x00,0xa0,0xc9,0x06,0x29,0xa8);
__CRT_UUID_DECL(IDirect3DDevice7,        0xf5049e79,0x4861,0x11d2,0xa4,0x07,0x00,0xa0,0xc9,0x06,0x29,0xa8);
__CRT_UUID_DECL(IDirect3DVertexBuffer7,  0xf5049e7d,0x4861,0x11d2,0xa4,0x07,0x00,0xa0,0xc9,0x06,0x29,0xa8);
#endif


// Undefine D3D7 macros
#undef DIRECT3D_VERSION
#undef D3D_SDK_VERSION

#undef D3DCS_ALL            // parentheses added in D3D9
#undef D3DFVF_POSITION_MASK // changed from 0x00E to 0x400E in D3D9
#undef D3DFVF_RESERVED2     // reduced from 4 to 2 in DX9

#undef D3DSP_REGNUM_MASK    // changed from 0x00000FFF to 0x000007FF in D3D9

#if defined(__MINGW32__) || defined(__GNUC__)

// Avoid redundant definitions (add D3D*_DEFINED macros here)
#define D3DRECT_DEFINED
#define D3DMATRIX_DEFINED

// Temporarily override __CRT_UUID_DECL to allow usage in d3d9 namespace
#pragma push_macro("__CRT_UUID_DECL")
#ifdef __CRT_UUID_DECL
#undef __CRT_UUID_DECL
#endif

#ifdef __MINGW32__
#define __CRT_UUID_DECL(type,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)                                                                               \
}                                                                                                                                           \
  extern "C++" template<> struct __mingw_uuidof_s<d3d9::type> { static constexpr IID __uuid_inst = {l,w1,w2, {b1,b2,b3,b4,b5,b6,b7,b8}}; }; \
  extern "C++" template<> constexpr const GUID &__mingw_uuidof<d3d9::type>() { return __mingw_uuidof_s<d3d9::type>::__uuid_inst; }          \
  extern "C++" template<> constexpr const GUID &__mingw_uuidof<d3d9::type*>() { return __mingw_uuidof_s<d3d9::type>::__uuid_inst; }         \
namespace d3d9 {

#elif defined(__GNUC__)
#define __CRT_UUID_DECL(type, a, b, c, d, e, f, g, h, i, j, k)                                                               \
}                                                                                                                            \
  extern "C++" { template <> constexpr GUID __uuidof_helper<d3d9::type>() { return GUID{a,b,c,{d,e,f,g,h,i,j,k}}; } }        \
  extern "C++" { template <> constexpr GUID __uuidof_helper<d3d9::type*>() { return __uuidof_helper<d3d9::type>(); } }       \
  extern "C++" { template <> constexpr GUID __uuidof_helper<const d3d9::type*>() { return __uuidof_helper<d3d9::type>(); } } \
  extern "C++" { template <> constexpr GUID __uuidof_helper<d3d9::type&>() { return __uuidof_helper<d3d9::type>(); } }       \
  extern "C++" { template <> constexpr GUID __uuidof_helper<const d3d9::type&>() { return __uuidof_helper<d3d9::type>(); } } \
namespace d3d9 {
#endif

#endif // defined(__MINGW32__) || defined(__GNUC__)


/**
* \brief Direct3D 9
*
* All D3D9 interfaces are included within
* a namespace, so as not to collide with
* D3D8 interfaces.
*/
namespace d3d9 {
#include <d3d9.h>
}

// Indicates d3d9:: namespace is in-use.
#define DXVK_D3D9_NAMESPACE

#if defined(__MINGW32__) || defined(__GNUC__)
#pragma pop_macro("__CRT_UUID_DECL")
#endif

//for some reason we need to specify __declspec(dllexport) for MinGW
#if defined(__WINE__) || !defined(_WIN32)
  #define DLLEXPORT __attribute__((visibility("default")))
#else
  #define DLLEXPORT
#endif


#include "../util/com/com_guid.h"
#include "../util/com/com_object.h"
#include "../util/com/com_pointer.h"

#include "../util/log/log.h"
#include "../util/log/log_debug.h"

#include "../util/sync/sync_recursive.h"

#include "../util/util_error.h"
#include "../util/util_likely.h"
#include "../util/util_string.h"

// Missed definitions in Wine/MinGW.

#ifndef D3DPRESENT_BACK_BUFFERS_MAX_EX
#define D3DPRESENT_BACK_BUFFERS_MAX_EX    30
#endif

#ifndef D3DSI_OPCODE_MASK
#define D3DSI_OPCODE_MASK                 0x0000FFFF
#endif

#ifndef D3DSP_TEXTURETYPE_MASK
#define D3DSP_TEXTURETYPE_MASK            0x78000000
#endif

#ifndef D3DUSAGE_AUTOGENMIPMAP
#define D3DUSAGE_AUTOGENMIPMAP            0x00000400L
#endif

#ifndef D3DSP_DCL_USAGE_MASK
#define D3DSP_DCL_USAGE_MASK              0x0000000f
#endif

#ifndef D3DSP_OPCODESPECIFICCONTROL_MASK
#define D3DSP_OPCODESPECIFICCONTROL_MASK  0x00ff0000
#endif

#ifndef D3DSP_OPCODESPECIFICCONTROL_SHIFT
#define D3DSP_OPCODESPECIFICCONTROL_SHIFT 16
#endif

#ifndef D3DCURSOR_IMMEDIATE_UPDATE
#define D3DCURSOR_IMMEDIATE_UPDATE        0x00000001L
#endif

#ifndef D3DPRESENT_FORCEIMMEDIATE
#define D3DPRESENT_FORCEIMMEDIATE         0x00000100L
#endif

// From d3dtypes.h

#ifndef D3DDEVINFOID_TEXTUREMANAGER
#define D3DDEVINFOID_TEXTUREMANAGER       1
#endif

#ifndef D3DDEVINFOID_D3DTEXTUREMANAGER
#define D3DDEVINFOID_D3DTEXTUREMANAGER    2
#endif

#ifndef D3DDEVINFOID_TEXTURING
#define D3DDEVINFOID_TEXTURING            3
#endif

// From d3dhal.h

#ifndef D3DDEVINFOID_VCACHE
#define D3DDEVINFOID_VCACHE               4
#endif

// MinGW headers are broken. Who'dve guessed?
#ifndef _MSC_VER

// Missing from d3d8types.h
#ifndef D3DDEVINFOID_RESOURCEMANAGER
#define D3DDEVINFOID_RESOURCEMANAGER      5
#endif

#ifndef D3DDEVINFOID_VERTEXSTATS
#define D3DDEVINFOID_VERTEXSTATS          6 // Aka D3DDEVINFOID_D3DVERTEXSTATS
#endif

#ifndef D3DPRESENT_RATE_UNLIMITED
#define D3DPRESENT_RATE_UNLIMITED         0x7FFFFFFF
#endif

#else // _MSC_VER

// These are enum typedefs in the MinGW headers, but not defined by Microsoft
#define D3DVSDT_TYPE                      DWORD
#define D3DVSDE_REGISTER                  DWORD

#endif
