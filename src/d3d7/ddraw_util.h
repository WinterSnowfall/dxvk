#pragma once

#include "d3d7_include.h"

#define GET_BIT_FIELD(token, field) ((token & field ## _MASK) >> field ## _SHIFT)

namespace dxvk {

  inline d3d9::D3DFORMAT ConvertFormat(DDPIXELFORMAT& fmt) {
    if (fmt.dwFlags & DDPF_RGB) {
      // R bitmask: 0111 1100 0000 0000
      // G bitmask: 0000 0011 1110 0000
      // B bitmask: 0000 0000 0001 1111
      switch (fmt.dwRGBBitCount) {
        case 16:
          return d3d9::D3DFMT_X1R5G5B5;
        case 32:
          return d3d9::D3DFMT_A8R8G8B8;
      }
    }
    return d3d9::D3DFMT_A8B8G8R8;
  }

  inline d3d9::D3DTRANSFORMSTATETYPE ConvertTransformState(D3DTRANSFORMSTATETYPE tst) {
    switch (tst) {
      case D3DTRANSFORMSTATE_WORLD:  return d3d9::D3DTRANSFORMSTATETYPE(D3DTS_WORLD);
      case D3DTRANSFORMSTATE_WORLD1: return d3d9::D3DTRANSFORMSTATETYPE(D3DTS_WORLD1);
      case D3DTRANSFORMSTATE_WORLD2: return d3d9::D3DTRANSFORMSTATETYPE(D3DTS_WORLD2);
      case D3DTRANSFORMSTATE_WORLD3: return d3d9::D3DTRANSFORMSTATETYPE(D3DTS_WORLD3);
      default: return d3d9::D3DTRANSFORMSTATETYPE(tst);
    }
  }

  inline size_t GetFVFSize(DWORD FVF) {
    size_t stride = 3;
    switch (FVF & D3DFVF_POSITION_MASK) {
      case D3DFVF_XYZRHW: stride += 1; break;
      case D3DFVF_XYZB1:  stride += 1; break;
      case D3DFVF_XYZB2:  stride += 2; break;
      case D3DFVF_XYZB3:  stride += 3; break;
      case D3DFVF_XYZB4:  stride += 4; break;
      case D3DFVF_XYZB5:  stride += 5; break;
    }
    if (FVF & D3DFVF_NORMAL)    stride += 3;
    if (FVF & D3DFVF_RESERVED1) stride += 1; // TODO: What is D3DFVF_RESERVED1? Point size?
    if (FVF & D3DFVF_DIFFUSE)   stride += 1;
    if (FVF & D3DFVF_SPECULAR)  stride += 1;

    if (FVF & D3DFVF_TEXCOUNT_MASK) {
      DWORD texCount = GET_BIT_FIELD(FVF, D3DFVF_TEXCOUNT);
      for (DWORD i = 0; i < texCount; i++) {
        if      ((FVF & D3DFVF_TEXCOORDSIZE1(i)) == D3DFVF_TEXCOORDSIZE1(i)) stride += 1;  // & 3
        else if ((FVF & D3DFVF_TEXCOORDSIZE3(i)) == D3DFVF_TEXCOORDSIZE3(i)) stride += 3;  // & 1
        else if ((FVF & D3DFVF_TEXCOORDSIZE4(i)) == D3DFVF_TEXCOORDSIZE4(i)) stride += 4;  // & 2
        else if ((FVF & D3DFVF_TEXCOORDSIZE2(i)) == D3DFVF_TEXCOORDSIZE2(i)) stride += 2;  // & 0 (default)
      }
    }
    return stride * sizeof(float);
  }

  inline unsigned GetPrimitiveSize(D3DPRIMITIVETYPE PrimitiveType) {
    switch (PrimitiveType) {
      case D3DPT_POINTLIST:     return 1;
      case D3DPT_LINELIST:      return 2;
      case D3DPT_LINESTRIP:     return 1;
      case D3DPT_TRIANGLELIST:  return 3;
      case D3DPT_TRIANGLESTRIP: return 3;
      case D3DPT_TRIANGLEFAN:   return 3;
      default: return 0;
    }
  }

  // If this D3DTEXTURESTAGESTATETYPE has been remapped to a d3d9::D3DSAMPLERSTATETYPE
  // it will be returned, otherwise returns -1
  // TODO: Verify.
  inline d3d9::D3DSAMPLERSTATETYPE ConvertSamplerStateType(const D3DTEXTURESTAGESTATETYPE StageType) {
    switch (StageType) {
      // 13-21:
      case D3DTSS_ADDRESSU:       return d3d9::D3DSAMP_ADDRESSU;
      case D3DTSS_ADDRESSV:       return d3d9::D3DSAMP_ADDRESSV;
      case D3DTSS_BORDERCOLOR:    return d3d9::D3DSAMP_BORDERCOLOR;
      case D3DTSS_MAGFILTER:      return d3d9::D3DSAMP_MAGFILTER;
      case D3DTSS_MINFILTER:      return d3d9::D3DSAMP_MINFILTER;
      case D3DTSS_MIPFILTER:      return d3d9::D3DSAMP_MIPFILTER;
      case D3DTSS_MIPMAPLODBIAS:  return d3d9::D3DSAMP_MIPMAPLODBIAS;
      case D3DTSS_MAXMIPLEVEL:    return d3d9::D3DSAMP_MIPFILTER;
      case D3DTSS_MAXANISOTROPY:  return d3d9::D3DSAMP_MAXANISOTROPY;
      default:                    return d3d9::D3DSAMPLERSTATETYPE(-1);
    }
  }

}