// 
// MIT License
// 
// Copyright (c) 2018 Marcus Larsson
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef win32_dx__h
#define win32_dx__h

#include <d3d11.h>
//#include <dxgi.h>

#ifdef DEBUG
#include <dxgidebug.h>
//#include <stdio.h>
#endif

//#include <dwrite.h> // DirectWrite (Dwrite.lib)
//#include <d2d1_1.h> // Direct2D    (D2d1_1.lib)
#include <vector> // @debug

//#include "mathematics.h"
#include "draw_calls.h"
#include "win32_dw.h"
#include "resources.h"
#include "bmp.h"




//
// Textures
//

struct dx_texture
{
    ID3D11Texture2D *Texture = nullptr;
    ID3D11ShaderResourceView *ShaderView = nullptr;
};

texture_index CreateTexture(void *DXState, bmp *BMP);
void Free(dx_texture *Texture);




//
// DirectX
//

struct shader_constants
{
    m4 WorldToClip;
    m4 ObjectToWorld;
    v4 Colour;
};


struct dx_state
{
    D3D11_VIEWPORT Viewport = {};
    
    HWND hWnd = nullptr;
    
    ID3D11Device *Device = nullptr;
    ID3D11DeviceContext *DeviceContext = nullptr;
    IDXGISwapChain *SwapChain = nullptr;
    ID3D11Texture2D *Backbuffer = nullptr;
    ID3D11RenderTargetView *BackbufferView = nullptr;
    ID3D11RasterizerState *RasterizerState = nullptr;
    
    //
    // Basic program (renders to RenderTargetTexture)
    ID3D11InputLayout *ilBasic = nullptr;
    ID3D11VertexShader *vsBasic = nullptr;
    ID3D11PixelShader *psBasic = nullptr;
    
    //
    // Primitives
    // @debug
    ID3D11Buffer *VertexBufferRectangle = nullptr;
    ID3D11Buffer *VertexBufferCircle = nullptr;
    u32 VertexCountCircle = 0;
    ID3D11Texture2D *PrimitiveTexture = nullptr; // A 1x1 white texture
    ID3D11ShaderResourceView *PrimitiveShaderView;
    
    //
    // Fullscreen program (renders the RenderTargetTexture to a textured fullscreen quad)
    ID3D11InputLayout *ilFullscreen = nullptr;
    ID3D11VertexShader *vsFullscreen = nullptr;
    ID3D11PixelShader *psFullscreen = nullptr;
    ID3D11Buffer *VertexBufferFullscreen = nullptr;
    
    //
    // Samplers
    ID3D11SamplerState *Sampler = nullptr;
    
    //
    // Render target stuff
    ID3D11Texture2D *RenderTargetTexture = nullptr; // MSAA texture
    ID3D11RenderTargetView *RenderTargetView = nullptr;
    ID3D11Texture2D *DepthStencilTexture = nullptr; // MSAA texture
    ID3D11DepthStencilView *DepthStencilView = nullptr;
    ID3D11DepthStencilState *DepthStencilState = nullptr;
    
    ID3D11Texture2D *ResolveTexture = nullptr; // The result of the MSAA resolve from the RenderTargetTexture
    ID3D11ShaderResourceView *ResolveShaderView = nullptr;
    
    //
    // Constant buffer
    shader_constants ShaderConstants = {};
    ID3D11Buffer *ShaderConstantsBuffer = nullptr;
    
    //
    // Config
    v4 BackgroundColour = V4(0.1f, 0.2f, 0.3f, 1.0f);
    
    u32 Width = 1920;
    u32 Height = 1080;
    
    //
    // DirectWrite
    dw_state DWState;
    
    
    //
    // Textures
    std::vector<dx_texture> Textures;
};

b32 Init(dx_state *State, HWND hWnd);
void Shutdown(dx_state *State);

void UpdateConstantBuffer(dx_state *State, shader_constants *ShaderConstants);
void ProcessDrawCalls(dx_state *State, draw_calls *DrawCalls);



#endif