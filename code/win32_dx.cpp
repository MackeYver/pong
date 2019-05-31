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

#include "win32_dx.h"
#include "win32_file_io.h"

#ifdef DEBUG
#include <stdio.h>
#include <assert.h>
#else
#define printf(...)
#define assert(x)
#endif



//
// Todo
// 
// + Add support for fullscreen
//   - IDXGIFactory::MakeWindowAssociation 
//



//
// Shutdown and Init
//
// TODO(Marcus): Unify and simpify the relase process... (Can we avoid having three different functions?).

#define DX_RELEASE(x) State->x ? State->x->Release() : 0
#define DX_SHUTDOWN(x) Shutdown(&State->x)
#define DX_FREE(x) Free(&State->x)

void Shutdown(dx_state *State)
{
    DX_RELEASE(RasterizerState);
    DX_RELEASE(BlendState);
    DX_RELEASE(DepthStencilState);
    DX_RELEASE(DepthStencilView);
    DX_RELEASE(DepthStencilTexture);
    DX_FREE(ResolveTexture);
    DX_RELEASE(RenderTargetView);
    DX_RELEASE(RenderTargetTexture);
    DX_SHUTDOWN(ShaderPrimitive);
    DX_SHUTDOWN(ShaderTextured);
	DX_SHUTDOWN(ShaderFinal);
    DX_RELEASE(BackbufferView);
    DX_RELEASE(Backbuffer);
    DX_RELEASE(SwapChain);
    DX_RELEASE(DeviceContext);
    DX_RELEASE(Device);
    
    Shutdown(&State->DWState);
    
    
    //
    // Textures
    for (std::vector<dx_texture>::size_type Index = 0;
         Index < State->Textures.size();
         ++Index)
    {
        Free(&State->Textures[Index]);
    }
    State->Textures.clear();
    
    //
    // Meshes
    for (std::vector<dx_mesh>::size_type Index = 0;
         Index < State->Meshes.size();
         ++Index)
    {
        Free(&State->Meshes[Index]);
    }
    State->Meshes.clear();
}


b32 Init(dx_state *State, HWND hWnd)
{
    HRESULT Result;
    State->hWnd = hWnd;
    
    
    
    
    //
    // Create device
    //
    
    {
        // This flag adds support for surfaces with a color-channel ordering different
        // from the API default. It is required for compatibility with Direct2D.
        // UINT DeviceFlags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
        // TODO(Marcus): Handle pixel format a bit more professionaly, check which formats that are
        //               supported etc...
        //               -- 2018-12-16
        //
        UINT DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        
#ifdef DEBUG
        // Requires D3D11_1SDKLayers.dll
        //DeviceFlags = DeviceFlags | D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DEBUGGABLE;
        DeviceFlags = DeviceFlags | D3D11_CREATE_DEVICE_DEBUG;
#endif
        
        
        //
        // Create device (default GPU)
        D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
        Result = D3D11CreateDevice(nullptr,                   // Adapter, nullptr = default
                                   D3D_DRIVER_TYPE_HARDWARE,  // Driver type
                                   nullptr,                   // HMODULE
                                   DeviceFlags,               // Flags
                                   &FeatureLevels,            // Feature level
                                   1,
                                   D3D11_SDK_VERSION,         // Version of the SDK
                                   &State->Device,            // Pointer to the device
                                   nullptr,                   // Returns the used feature level (we don't need that)
                                   &State->DeviceContext);
        if (FAILED(Result) || !State->Device)
        {
            printf("Failed to create device!\n");
            return false;
        }
    }
    
    
    
    // @debug
    u32 SampleDescCount = 4; //CheckMultisampleQualityLevels
    u32 SampleDescQuality = 0;
    
    
    
    //
    // Create SwapChain
    //
    
    {
        //
        // Fill out a structure describing the SwapChain
        DXGI_SWAP_CHAIN_DESC SwapChainDesc;
        ZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
        SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        SwapChainDesc.BufferDesc.Width = State->Width;
        SwapChainDesc.BufferDesc.Height = State->Height;
        SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        
        SwapChainDesc.SampleDesc.Count = 1; // multisampling setting
        SwapChainDesc.SampleDesc.Quality = 0; // vendor-specific flag
        
        SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        SwapChainDesc.BufferCount = 2;
        SwapChainDesc.OutputWindow = State->hWnd;
        SwapChainDesc.Windowed = true;           
        SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // The Flip model is probably better but that requires win 8
        //SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // @compatibility, requires at least windows 8
        SwapChainDesc.Flags = 0;
        
        
        //
        // Create the SwapChain
        IDXGIFactory *Factory;
        Result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory);
        if (FAILED(Result)) 
        {
            printf("Failed to create DXGIFactory!\n");
            return false;
        }
        
        Result = Factory->CreateSwapChain(State->Device, &SwapChainDesc, &State->SwapChain);
        if (FAILED(Result)) 
        {
            printf("Failed to create the swap chain!\n");
            return false;
            
        }
        
        Factory->Release();
        
        
        //
        // Get a pointer to the backbuffer
        Result = State->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&State->Backbuffer);
        if (FAILED(Result)) 
        {
            printf("Failed to get the backbuffer!\n");
            return false;
        }
        
        
        //
        // RenderTargetView
        Result = State->Device->CreateRenderTargetView(State->Backbuffer, nullptr, &State->BackbufferView);
        if (FAILED(Result)) 
        {
            printf("Failed to create the RenderTargetView!\n");
            return false;
        }
    }
    
    
    
    
    //
    // Viewport
    //
    
    {
        State->Viewport.TopLeftX = 0;
        State->Viewport.TopLeftY = 0;
        State->Viewport.Width    = (f32)State->Width;
        State->Viewport.Height   = (f32)State->Height;
        State->Viewport.MinDepth = 0;
        State->Viewport.MaxDepth = 1;
    }
    
    
    
    //
    // Rasterizer state
    // 
    {
        D3D11_RASTERIZER_DESC Desc;
        ZeroMemory(&Desc, sizeof(Desc));
        Desc.FillMode = D3D11_FILL_SOLID;
        Desc.CullMode = D3D11_CULL_BACK;
        Desc.FrontCounterClockwise = true;
        Desc.DepthClipEnable = true;
        Desc.MultisampleEnable = true;
        Desc.AntialiasedLineEnable = true;
        
        Result = State->Device->CreateRasterizerState(&Desc, &State->RasterizerState);
        if (FAILED(Result))
        {
            printf("Failed to create solid rasterizer state!\n");
            return false;
        }
    }
    
    
    
    
    //
    // Shaders
    //
    
    {
        //
        // Primitive shader
        {
            b32 bResult = Init(State->Device, &State->ShaderPrimitive);
            if (!bResult)
            {
                printf("%s: failed to create shader program named: shader_primitive.\n", __FILE__);
                return false;
            }
            
            f32 w = 1.0f / (0.5f * State->Width);
            f32 h = 1.0f / (0.5f * State->Height);
            State->ShaderPrimitive.Constants.WorldToClip = M4Scale(w, h, 1.0f) * M4Translation(-1.0f, -1.0f, 0.0f);
            UpdateConstants(State->DeviceContext, &State->ShaderPrimitive);
        }
        
        
        //
        // Textured mesh
        {
            b32 bResult = Init(State->Device, &State->ShaderTextured);
            if (!bResult)
            {
                printf("%s: failed to create shader program named: shader_textured.\n", __FILE__);
                return false;
            }
            
            f32 w = 1.0f / (0.5f * State->Width);
            f32 h = 1.0f / (0.5f * State->Height);
            State->ShaderTextured.Constants.WorldToClip = M4Scale(w, h, 1.0f) * M4Translation(-1.0f, -1.0f, 0.0f);
        }
        
        
        //
        // Final render, renders the resolve texture to the screen
        {
            b32 bResult = Init(State->Device, &State->ShaderFinal);
            if (!bResult)
            {
                printf("%s: failed to create shader program named: shader_final.\n", __FILE__);
                return false;
            }
        }
    }
    
    
    
    
    //
    // Render target, a texture that we will render to, which will be resolved into ResolveTexture, which in turn
    // will be rendered on a fullscreen quad to the Backbuffer (final render target).
    // This is MSAA enabled (the resolve texture is not).
    //
    
    {
        //
        // Texture
        D3D11_TEXTURE2D_DESC TexDesc;
        ZeroMemory(&TexDesc, sizeof(D3D11_TEXTURE2D_DESC));
        
        TexDesc.Width = State->Width;
        TexDesc.Height = State->Height;
        TexDesc.MipLevels = 1;
        TexDesc.ArraySize = 1;
        TexDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        TexDesc.SampleDesc.Count = SampleDescCount;
        TexDesc.SampleDesc.Quality = SampleDescQuality;
        TexDesc.Usage = D3D11_USAGE_DEFAULT;
        TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
        TexDesc.CPUAccessFlags = 0;
        TexDesc.MiscFlags = 0;
        
        Result = State->Device->CreateTexture2D(&TexDesc, nullptr, &State->RenderTargetTexture);
        if (FAILED(Result)) 
        {
            printf("Failed to create the texture for the render_target.\n");
            return false;
        }
        
        
        //
        // View
        D3D11_RENDER_TARGET_VIEW_DESC ViewDesc;
        ViewDesc.Format = TexDesc.Format;
        ViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS; // ABBA
        ViewDesc.Texture2D.MipSlice = 0;
        
        Result = State->Device->CreateRenderTargetView(State->RenderTargetTexture, &ViewDesc, &State->RenderTargetView);
        if (FAILED(Result)) 
        {
            printf("Failed to create the view for the render_target.\n");
            return false;
        }
    }
    
    
    
    
    //
    // Depth-/stencil-stuff
    //
    
    {
        //
        // Create the texture for the depth-/stencil-buffer
        D3D11_TEXTURE2D_DESC TexDesc;
        ZeroMemory(&TexDesc, sizeof(D3D11_TEXTURE2D_DESC));
        State->RenderTargetTexture->GetDesc(&TexDesc);
        TexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        TexDesc.Usage = D3D11_USAGE_DEFAULT;
        TexDesc.SampleDesc.Count = SampleDescCount;
        TexDesc.SampleDesc.Quality = SampleDescQuality;
        TexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        TexDesc.MiscFlags = 0;
        
        Result = State->Device->CreateTexture2D(&TexDesc, nullptr, &State->DepthStencilTexture);
        if (FAILED(Result)) 
        {
            printf("Failed to create the depth-/stencil-buffer!\n");
            return false;
        }
        
        
        D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;
        DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        DepthStencilViewDesc.Texture2D.MipSlice = 0;
        DepthStencilViewDesc.Flags = 0;
        
        Result = State->Device->CreateDepthStencilView(State->DepthStencilTexture, 
                                                       &DepthStencilViewDesc, 
                                                       &State->DepthStencilView);
        if (FAILED(Result)) 
        {
            printf("Failed to create the depth-/stencil-view!\n");
            return false;
        }
        
        
        //
        // Depth-/stencil-state
        D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
        DepthStencilDesc.DepthEnable = true;
        DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        DepthStencilDesc.DepthFunc      = D3D11_COMPARISON_LESS;
        //DepthStencilDesc.DepthFunc      = D3D11_COMPARISON_ALWAYS;
        
        // Stencil test parameters
        DepthStencilDesc.StencilEnable = false;
        DepthStencilDesc.StencilReadMask = 0xFF;
        DepthStencilDesc.StencilWriteMask = 0xFF;
        
        // Stencil operations if pixel is front-facing
        DepthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
        DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        DepthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
        DepthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
        
        // Stencil operations if pixel is back-facing
        DepthStencilDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
        DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        DepthStencilDesc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
        DepthStencilDesc.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
        
        // Create depth stencil state
        Result = State->Device->CreateDepthStencilState(&DepthStencilDesc, &State->DepthStencilState);
        if (FAILED(Result)) 
        {
            printf("Failed to create the depth-/stencil-state!\n");
            return false;
        }
    }
    
    
    
    
    //
    // Blend state
    //
    
    {
        // color = alpha * src + (1 - alpha) * dest
        D3D11_RENDER_TARGET_BLEND_DESC RenderTargetBlendDesc;
        ZeroMemory(&RenderTargetBlendDesc, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
        RenderTargetBlendDesc.BlendEnable = true;
        RenderTargetBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        RenderTargetBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        RenderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
        RenderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
        RenderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
        RenderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        RenderTargetBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        
        D3D11_BLEND_DESC BlendDesc;
        ZeroMemory(&BlendDesc, sizeof(D3D11_BLEND_DESC));
        BlendDesc.AlphaToCoverageEnable = false;
        BlendDesc.IndependentBlendEnable = false;
        BlendDesc.RenderTarget[0] = RenderTargetBlendDesc;
        
        
        Result = State->Device->CreateBlendState(&BlendDesc, &State->BlendState);
        if (FAILED(Result)) 
        {
            printf("Failed to create the blend state!\n");
            return false;
        }
    }
    
    
    
    
    //
    // Rensolve target, a texture that we will be the result of a MSAA resolve from RenderTargetTexture
    // This will be rendered on a fullscreen quad to the Backbuffer (final render target).
    // NOT MSAA.
    //
    
    {
        b32 bResult = CreateTexture(State->Device, State->Width, State->Height, &State->ResolveTexture);
        if (!bResult) 
        {
            printf("%s: Failed to create the resolve texture.\n", __FILE__);
            return false;
        }
    }
    
    
    
    
    //
    // Use some of the stuff that was created above
    //
    
    State->DeviceContext->RSSetViewports(1, &State->Viewport);
    // The shaders are set in the Begin-/End-Rendering functions
    // Also the rendertargets are set in the Begin-/End-Rendering functions
    State->DeviceContext->RSSetState(State->RasterizerState);
    
    
    //
    // DirectWrite
    Init(&State->DWState, State);
    
    
    return true;
}




//
// Ante-/post-processing
// 

void BeginRendering(dx_state *State)
{
    //
    // This function will set us up for rendering to the RenderTargetTexture, which is a MSAA texture.
    //
    
    ID3D11DeviceContext *DC = State->DeviceContext;
    
    //
    // Blending
    DC->OMSetBlendState(State->BlendState, nullptr, 0xFFFFFFFF);
    
    //
    // Clear the depth-/stencil-view and the texture used as render target
    DC->ClearDepthStencilView(State->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    DC->ClearRenderTargetView(State->RenderTargetView, (f32 *)&State->BackgroundColour);
    
    //
    // Set the RenderTargetTexture as the render target
    DC->OMSetDepthStencilState(State->DepthStencilState, 1);
    DC->OMSetRenderTargets(1, &State->RenderTargetView, State->DepthStencilView);
}


void EndRendering(dx_state *State)
{
    //
    // This function will resolve the RenderTargetTexture, which is a MSAA texture.
    // The RenderTargetTexture will be resolved into ResolveTexture, which in turn will be rendered 
    // onto a quad into the Backbuffer (which will be the final render target and is _not_ MSAA enabled).
    //
    
    ID3D11DeviceContext *DC = State->DeviceContext;
    
    //
    // Resolve
    DC->ResolveSubresource(State->ResolveTexture.Texture, 0,
                           State->RenderTargetTexture, 0,
                           DXGI_FORMAT_B8G8R8A8_UNORM);
    
    //
    // No blending
    DC->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    
    
    //
    // Set the backbuffer (in the swap chain) as the render target
    State->DeviceContext->OMSetDepthStencilState(nullptr, 1);
    State->DeviceContext->OMSetRenderTargets(1, &State->BackbufferView, nullptr);
    
    
    //
    // Use the shader that will render a texture to the screen
    Use(DC, &State->ShaderFinal);
    
    //
    // Draw
    DrawTextureToScreen(DC, &State->ShaderFinal, &State->ResolveTexture);
    
    //
    // Update
    State->SwapChain->Present(1, 0); // vertical sync enabled if the first param is 1
}




//
// Draw calls
//

void RenderTexturedMesh(dx_state *State, m4 ObjectToWorld, s32 MeshIndex, s32 TextureIndex, v4 Colour)
{
    ID3D11DeviceContext *DC = State->DeviceContext;
    
    State->ShaderTextured.Constants.Colour = Colour;
    State->ShaderTextured.Constants.ObjectToWorld = ObjectToWorld;
    UpdateConstants(DC, &State->ShaderTextured);
    
    Use(DC, &State->ShaderTextured);
    
    dx_mesh *Mesh = &State->Meshes[MeshIndex];
    dx_texture *Texture = &State->Textures[TextureIndex];
    DrawMesh(DC, &State->ShaderTextured, Mesh, Texture);
}




//
// Process draw_calls
//

void ProcessDrawCalls(dx_state *State, draw_calls *DrawCalls)
{
    BeginRendering(State);
    
    
    //
    // Process primitives
    if (DrawCalls->LineCount > 0)
    {
        ID3D11DeviceContext *DC = State->DeviceContext;
        
        Use(DC, &State->ShaderPrimitive);
        DrawPrimitives(State->Device, DC, &State->ShaderPrimitive, D3D_PRIMITIVE_TOPOLOGY_LINELIST, 
                       &DrawCalls->PrimitiveLinesMemory, 2 * DrawCalls->LineCount);
    }
    
    if (DrawCalls->TriangleCount > 0)
    {
        ID3D11DeviceContext *DC = State->DeviceContext;
        
        Use(DC, &State->ShaderPrimitive);
        DrawPrimitives(State->Device, DC, &State->ShaderPrimitive, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 
                       &DrawCalls->PrimitiveTrianglesMemory, 3 * DrawCalls->TriangleCount);
    }
    
    
    //
    // Process draw calls
    for (u8 *CurrAddress = DrawCalls->Memory.Ptr;
         CurrAddress < (DrawCalls->Memory.Ptr + DrawCalls->Memory.Used);
         )
    {
        draw_call_header *Header = reinterpret_cast<draw_call_header *>(CurrAddress);
        
        switch (Header->Type)
        {
            case DrawCallType_Text:
            {
                draw_call_text *DrawCall = reinterpret_cast<draw_call_text *>(CurrAddress);
                DrawCall;
                
                BeginDraw(&State->DWState);
                DrawText(&State->DWState, DrawCall->P, DrawCall->Text, DrawCall->SizeIndex, DrawCall->ColourIndex);
                EndDraw(&State->DWState);
            } break;
            
            case DrawCallType_TexturedMesh:
            {
                draw_call_textured_mesh *DrawCall = reinterpret_cast<draw_call_textured_mesh *>(CurrAddress);;
                RenderTexturedMesh(State, DrawCall->ObjectToWorld, DrawCall->MeshIndex, DrawCall->TextureIndex, DrawCall->Colour);
            } break;
            
            default:
            {
                assert(0);
            } break;
        }
        
        CurrAddress += Header->Size;
    }
    
    
    //
    // Done!
    ClearMemory(DrawCalls);
    EndRendering(State);
}