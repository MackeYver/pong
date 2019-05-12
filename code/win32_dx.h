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
#endif

#include <vector> // @debug

#include "mathematics.h"
#include "draw_calls.h"
#include "win32_dw.h"

#include "shader_primitive.h"
#include "shader_textured.h"
#include "shader_final.h"

#include "win32_dx_buffer.h"
#include "win32_dx_texture.h"
#include "win32_dx_mesh.h"



//
// DirectX
//

struct dx_state
{
    D3D11_VIEWPORT Viewport = {};
    
    HWND hWnd = nullptr;
    
    ID3D11Device *Device = nullptr;
    ID3D11DeviceContext *DeviceContext = nullptr;
    
    ID3D11RasterizerState *RasterizerState = nullptr;
    ID3D11BlendState *BlendState;
    
    IDXGISwapChain *SwapChain = nullptr;
    ID3D11Texture2D *Backbuffer = nullptr;
    ID3D11RenderTargetView *BackbufferView = nullptr;
    
    
    //
    // Shaders
    shader_primitive ShaderPrimitive; // Primitive program (renders uniformed coloured primitives)
    shader_textured ShaderTextured;   // Basic program (renders to RenderTargetTexture)
    
    shader_final ShaderFinal;
    dx_texture ResolveTexture;
    
    //
    // Render target stuff
    ID3D11Texture2D *RenderTargetTexture = nullptr; // MSAA texture
    ID3D11RenderTargetView *RenderTargetView = nullptr;
    ID3D11Texture2D *DepthStencilTexture = nullptr; // MSAA texture
    ID3D11DepthStencilView *DepthStencilView = nullptr;
    ID3D11DepthStencilState *DepthStencilState = nullptr;
    
    
    //
    // Config
    v4 BackgroundColour = V4(0.0f, 0.0f, 0.0f, 1.0f);
    
    u32 Width = 1920;
    u32 Height = 1080;
    
    //
    // DirectWrite
    dw_state DWState;
    
    
    //
    // Textures
    std::vector<dx_texture> Textures;
    std::vector<dx_mesh> Meshes;
};

b32 Init(dx_state *State, HWND hWnd);
void Shutdown(dx_state *State);

void ProcessDrawCalls(dx_state *State, draw_calls *DrawCalls);



#endif