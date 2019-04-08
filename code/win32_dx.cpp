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
#else
#define printf(...)
#endif



//
// Rendering
// 

void BeginRendering(dx_state *State)
{
    ID3D11DeviceContext *DC = State->DeviceContext;
    
    //
    // Clear the depth-/stencil-view and the texture used as render target
    DC->ClearDepthStencilView(State->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    DC->ClearRenderTargetView(State->RenderTargetView, (float const *)&State->BackgroundColour);
    
    //
    // Set the RenderTargetTexture as the render target
    DC->OMSetDepthStencilState(State->DepthStencilState, 1);
    DC->OMSetRenderTargets(1, &State->RenderTargetView, State->DepthStencilView);
    
    //
    // Set shaders and resources
    DC->VSSetShader(State->vsBasic, nullptr, 0);
    DC->PSSetShader(State->psBasic, nullptr, 0);
    
    DC->VSSetConstantBuffers(0, 1, &State->ShaderConstantsBuffer);
    DC->PSSetConstantBuffers(0, 1, &State->ShaderConstantsBuffer);
    DC->GSSetConstantBuffers(0, 1, &State->ShaderConstantsBuffer);
}


void EndRendering(dx_state *State)
{
    ID3D11DeviceContext *DC = State->DeviceContext;
    
    //
    // Set the backbuffer (in the swap chain) as the render target
    State->DeviceContext->OMSetDepthStencilState(nullptr, 1);
    State->DeviceContext->OMSetRenderTargets(1, &State->BackbufferView, nullptr);
    
    //
    // Set shaders
    DC->VSSetShader(State->vsFullscreen, nullptr, 0);
    DC->PSSetShader(State->psFullscreen, nullptr, 0);
    
    State->DeviceContext->PSSetShaderResources(0, 1, &State->RenderTargetShaderView);
    State->DeviceContext->PSSetSamplers(0, 1, &State->Sampler);
    
    //
    // Render the fullscreen quad with the RenderTargetTexture as texture
    size_t Stride = sizeof(v4);
    size_t Offset = 0;
    DC->IASetVertexBuffers(0, 1, &State->VertexBufferFullscreen, &Stride, &Offset);
    DC->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DC->Draw(6, 0);
    
    ID3D11ShaderResourceView *NullView = nullptr;
    State->DeviceContext->PSSetShaderResources(0, 1, &NullView);
    
    //
    // Update
    State->SwapChain->Present(0, 0);
}




//
// Shutdown and Init
//

#define RELEASE(x) State->x ? State->x->Release() : 0;

void Shutdown(dx_state *State)
{
    RELEASE(VertexBufferFullscreen);
    RELEASE(ShaderConstantsBuffer);
    RELEASE(RasterizerState);
    RELEASE(DepthStencilState);
    RELEASE(DepthStencilView);
    RELEASE(DepthStencilTexture);
    RELEASE(RenderTargetShaderView);
    RELEASE(RenderTargetView);
    RELEASE(RenderTargetTexture);
    RELEASE(Sampler);
    RELEASE(psFullscreen);
    RELEASE(ilFullscreen);
    RELEASE(vsFullscreen);
    RELEASE(psBasic);
    RELEASE(ilBasic);
    RELEASE(vsBasic);
    RELEASE(BackbufferView);
    RELEASE(Backbuffer);
    RELEASE(SwapChain);
    RELEASE(DeviceContext);
    RELEASE(Device);
}


b32 Init(dx_state *State)
{
    //
    // Create device
    //
    
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
    HRESULT Result = D3D11CreateDevice(nullptr,                   // Adapter, nullptr = default
                                       D3D_DRIVER_TYPE_HARDWARE,  // Driver type
                                       nullptr,                   // HMODULE
                                       DeviceFlags,               // Flags
                                       &FeatureLevels,            // Feature level
                                       1,
                                       D3D11_SDK_VERSION,         // Version of the SDK
                                       &State->Device,                   // Pointer to the device
                                       nullptr,                   // Returns the used feature level (we don't need that)
                                       &State->DeviceContext);
    if (FAILED(Result) || !State->Device)
    {
        printf("Failed to create device!\n");
        return false;
    }
    
    
    
    //
    // Create SwapChain
    //
    
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
    
    SwapChainDesc.SampleDesc.Count = 1;      // multisampling setting
    SwapChainDesc.SampleDesc.Quality = 0;    // vendor-specific flag
    
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.OutputWindow = State->hWnd;
    SwapChainDesc.Windowed = true;           
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // The Flip model is probably better but that requires win 10
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
    
    
    
    //
    // Viewport
    //
    
    State->Viewport.TopLeftX = 0;
    State->Viewport.TopLeftY = 0;
    State->Viewport.Width    = (f32)State->Width;
    State->Viewport.Height   = (f32)State->Height;
    State->Viewport.MinDepth = 0;
    State->Viewport.MaxDepth = 1;
    
    
    
    //
    // Shaders
    //
    
    //
    // Basic shader (no texture or lighting)
    D3D11_INPUT_ELEMENT_DESC InputElements = {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0};
    
    // Vertex shader
    u8 *Data = nullptr;
    size_t DataSize = 0;
    win32_ReadFile("build\\shaders\\vbasic.cso", &Data, &DataSize);
    assert(Data);
    assert(DataSize > 0);
    
    Result = State->Device->CreateVertexShader(Data,
                                               DataSize,
                                               nullptr,
                                               &State->vsBasic);
    if (FAILED(Result)) {
        printf("Failed to create vertex shader from file!\n");
        return false;
    }
    
    // Input layout
    Result = State->Device->CreateInputLayout(&InputElements, 1, Data, DataSize, &State->ilBasic);
    
    if (Data)
    {
        free(Data);
    }
    
    // Pixel shader
    win32_ReadFile("build\\shaders\\pbasic.cso", &Data, &DataSize);
    assert(Data);
    assert(DataSize > 0);
    
    // Create shader
    Result = State->Device->CreatePixelShader(Data,
                                              DataSize,
                                              nullptr,
                                              &State->psBasic);
    if (Data)
    {
        free(Data);
    }
    
    if (FAILED(Result)) {
        printf("Failed to create pixel shader from file!\n");
        return false;
    }
    
    
    
    //
    // Render a textured quad, fullscreen
    D3D11_INPUT_ELEMENT_DESC InputElements2[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    
    // vertex shader
    win32_ReadFile("build\\shaders\\vfullscreen_texture.cso", &Data, &DataSize);
    assert(Data);
    assert(DataSize > 0);
    
    // Create shader
    Result = State->Device->CreateVertexShader(Data,
                                               DataSize,
                                               nullptr,
                                               &State->vsFullscreen);
    if (FAILED(Result)) {
        printf("Failed to create pixel shader from file!\n");
        return false;
    }
    
    // Input layout
    Result = State->Device->CreateInputLayout(InputElements2, 2, Data, DataSize, &State->ilFullscreen);
    
    if (Data)
    {
        free(Data);
    }
    
    // Pixel shader
    win32_ReadFile("build\\shaders\\pfullscreen_texture.cso", &Data, &DataSize);
    assert(Data);
    assert(DataSize > 0);
    
    // Create shader
    Result = State->Device->CreatePixelShader(Data,
                                              DataSize,
                                              nullptr,
                                              &State->psFullscreen);
    if (Data)
    {
        free(Data);
        DataSize = 0;
    }
    
    if (FAILED(Result)) {
        printf("Failed to create pixel shader from file!\n");
        return false;
    }
    
    
    //
    // Sampler
    //
    
    D3D11_SAMPLER_DESC SamplerDesc;
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.MipLODBias = 0.0f;
    SamplerDesc.MaxAnisotropy = 1;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    SamplerDesc.BorderColor[0] = 1.0f;
    SamplerDesc.BorderColor[1] = 0.0f;
    SamplerDesc.BorderColor[2] = 1.0f;
    SamplerDesc.BorderColor[3] = 1.0f;
    SamplerDesc.MinLOD = 0.0f;
    SamplerDesc.MaxLOD = 0.0f;
    
    Result = State->Device->CreateSamplerState(&SamplerDesc, &State->Sampler);
    if (FAILED(Result))
    {
        printf("Failed to create sampler!\n");
        return false;
    }
    
    
    
    
    //
    // Render target, a texture that we will render to, which will later be used as a texture on a fullscreen quad.
    //
    //
    // Texture
    D3D11_TEXTURE2D_DESC TexDesc;
    ZeroMemory(&TexDesc, sizeof(D3D11_TEXTURE2D_DESC));
    
    TexDesc.Width = State->Width;
    TexDesc.Height = State->Height;
    TexDesc.MipLevels = 1;
    TexDesc.ArraySize = 1;
    TexDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;;
    TexDesc.SampleDesc.Count = 1;
    TexDesc.SampleDesc.Quality = 0;
    TexDesc.Usage = D3D11_USAGE_DEFAULT;
    TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
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
    ViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    ViewDesc.Texture2D.MipSlice = 0;
    
    Result = State->Device->CreateRenderTargetView(State->RenderTargetTexture, &ViewDesc, &State->RenderTargetView);
    if (FAILED(Result)) 
    {
        printf("Failed to create the view for the render_target.\n");
        return false;
    }
    
    
    //
    // Shader view
    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderViewDesc;
    ShaderViewDesc.Format = TexDesc.Format;
    ShaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ShaderViewDesc.Texture2D.MostDetailedMip = 0;
    ShaderViewDesc.Texture2D.MipLevels = 1;
    
    Result = State->Device->CreateShaderResourceView(State->RenderTargetTexture, 
                                                     &ShaderViewDesc, 
                                                     &State->RenderTargetShaderView);
    if (FAILED(Result)) 
    {
        printf("Failed to create the shader's view for the render_target.\n");
        return false;
    }
    
    
    
    //
    // Create the texture for the depthstencilbuffer
    ZeroMemory(&TexDesc, sizeof(D3D11_TEXTURE2D_DESC));
    State->RenderTargetTexture->GetDesc(&TexDesc);
    TexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    TexDesc.Usage = D3D11_USAGE_DEFAULT;
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
    DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
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
    //DepthStencilDesc.DepthFunc      = D3D11_COMPARISON_LESS;
    DepthStencilDesc.DepthFunc      = D3D11_COMPARISON_ALWAYS;
    
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
    
    
    
    //
    // Rasterizer state
    // 
    
    D3D11_RASTERIZER_DESC Desc;
    ZeroMemory(&Desc, sizeof(Desc));
    Desc.FillMode = D3D11_FILL_SOLID;
    Desc.CullMode = D3D11_CULL_BACK;
    Desc.FrontCounterClockwise = true;
    Desc.DepthClipEnable = true;
    Desc.MultisampleEnable = false;
    Desc.AntialiasedLineEnable = true;
    
    Result = State->Device->CreateRasterizerState(&Desc, &State->RasterizerState);
    if (FAILED(Result))
    {
        printf("Failed to create solid rasterizer state!\n");
        return false;
    }
    
    
    
    //
    // Constant buffer
    //
    
    f32 w = 1.0f / (0.5f * State->Width);
    f32 h = 1.0f / (0.5f * State->Height);
    State->ShaderConstants.WorldToClip = M4Scale(w, h, 1.0f) * M4Translation(-1.0f, -1.0f, 0.0f);
    State->ShaderConstants.Colour = V4(0.65f, 0.65f, 0.7f, 1.0f);
    assert(sizeof(shader_constants) % 16 == 0);
    
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    BufferDesc.ByteWidth = sizeof(State->ShaderConstants);             
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;  // only usable by the GPU
    BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;       
    BufferDesc.MiscFlags = 0;                // No other option
    BufferDesc.StructureByteStride = 0;
    
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
    InitData.pSysMem = &State->ShaderConstants;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    
    Result = State->Device->CreateBuffer(&BufferDesc, &InitData, &State->ShaderConstantsBuffer);
    if (FAILED(Result)) 
    {
        printf("Failed to create the buffer\n");
        return false;
    }
    
    
    
    //
    //
    // Fullscreen quad
    struct t
    {
        v2 P;
        v2 T;
    };
    
    t Vertices[] =
    {
        {{-1.0f, -1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f}, {0.0f, 0.0f}},
        
        {{-1.0f, -1.0f}, {0.0f, 1.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 0.0f}},
    };
    
    u32 VertexCount = 6;
    size_t VertexSize = sizeof(t);
    size_t Size = VertexCount * VertexSize;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    BufferDesc.ByteWidth = Size;                     // size of the buffer
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;          // only usable by the GPU
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // use in vertex shader
    BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
    BufferDesc.MiscFlags = 0;                        // No other option
    BufferDesc.StructureByteStride = VertexSize;
    
    ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
    InitData.pSysMem = Vertices;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    
    Result = State->Device->CreateBuffer(&BufferDesc, &InitData, &State->VertexBufferFullscreen);
    if (FAILED(Result)) 
    {
        printf("Failed to create the VertexbufferFullscreen\n");
        return false;
    }
    
    
    
    
    //
    // Use all the stuff that was created above
    //
    
    State->DeviceContext->RSSetViewports(1, &State->Viewport);
    // The shaders are set in the Begin-/End-Rendering functions
    // Also the rendertargets are set in the Begin-/End-Rendering functions
    State->DeviceContext->RSSetState(State->RasterizerState);
    
    
    return true;
}