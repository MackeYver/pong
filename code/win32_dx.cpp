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
// Todo
// 
// + Add support for fullscreen
//   - IDXGIFactory::MakeWindowAssociation 
//



//
// Shutdown and Init
//

#define _RELEASE(x) State->x ? State->x->Release() : 0;

void Shutdown(dx_state *State)
{
    _RELEASE(VertexBufferCircle);
    _RELEASE(VertexBufferRectangle);
    _RELEASE(VertexBufferFullscreen);
    _RELEASE(ShaderConstantsBuffer);
    _RELEASE(RasterizerState);
    _RELEASE(DepthStencilState);
    _RELEASE(DepthStencilView);
    _RELEASE(DepthStencilTexture);
    _RELEASE(ResolveShaderView);
    _RELEASE(ResolveTexture);
    _RELEASE(RenderTargetView);
    _RELEASE(RenderTargetTexture);
    _RELEASE(Sampler);
    _RELEASE(psFullscreen);
    _RELEASE(ilFullscreen);
    _RELEASE(vsFullscreen);
    _RELEASE(psBasic);
    _RELEASE(ilBasic);
    _RELEASE(vsBasic);
    _RELEASE(BackbufferView);
    _RELEASE(Backbuffer);
    _RELEASE(SwapChain);
    _RELEASE(DeviceContext);
    _RELEASE(Device);
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
    
    
    
    // @debug ABBA1
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
        Desc.MultisampleEnable = false;
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
    }
    
    
    
    //
    // Sampler
    //
    {
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
    }
    
    
    
    //
    // Render target, a texture that we will render to, which will be resolved into ResolveTexture, which in turn
    // will be rendered on a fullscreen quad to the Backbuffer (final render target).
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
    // Rensolve target, a texture that we will be the result of a MSAA resolve from RenderTargetTexture
    // This will be rendered on a fullscreen quad to the Backbuffer (final render target).
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
        TexDesc.SampleDesc.Count = 1;
        TexDesc.SampleDesc.Quality = 0;
        TexDesc.Usage = D3D11_USAGE_DEFAULT;
        TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        TexDesc.CPUAccessFlags = 0;
        TexDesc.MiscFlags = 0;
        
        Result = State->Device->CreateTexture2D(&TexDesc, nullptr, &State->ResolveTexture);
        if (FAILED(Result)) 
        {
            printf("Failed to create the resolve texture.\n");
            return false;
        }
        
        
        //
        // Shader view
        D3D11_SHADER_RESOURCE_VIEW_DESC ShaderViewDesc;
        ShaderViewDesc.Format = TexDesc.Format;
        ShaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        ShaderViewDesc.Texture2D.MostDetailedMip = 0;
        ShaderViewDesc.Texture2D.MipLevels = 1;
        
        Result = State->Device->CreateShaderResourceView(State->ResolveTexture, 
                                                         &ShaderViewDesc, 
                                                         &State->ResolveShaderView);
        if (FAILED(Result)) 
        {
            printf("Failed to create the shader's view for the resolve texture.\n");
            return false;
        }
    }
    
    
    
    //
    // Constant buffer
    //
    {
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
    }
    
    
    
    //
    // Fullscreen quad, used to render the RenderTargetTexture to the screen
    //
    {
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
        
        D3D11_BUFFER_DESC BufferDesc;
        ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
        BufferDesc.ByteWidth = Size;                     // size of the buffer
        BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;        // This buffer will never change
        BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // use in vertex shader
        BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
        BufferDesc.MiscFlags = 0;                        // No other option
        BufferDesc.StructureByteStride = VertexSize;
        
        D3D11_SUBRESOURCE_DATA InitData;
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
    }
    
    
    
    //
    // Vertex buffer for a 1x1 rectangle
    // - @debug
    {
        v2 Vertices[] =
        {
            {-0.5f, -0.5f},
            { 0.5f,  0.5f},
            {-0.5f,  0.5f},
            
            {-0.5f, -0.5f},
            { 0.5f, -0.5f},
            { 0.5f,  0.5f},
        };
        
        u32 VertexCount = 6;
        size_t VertexSize = sizeof(v2);
        size_t Size = VertexCount * VertexSize;
        
        D3D11_BUFFER_DESC BufferDesc;
        ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
        BufferDesc.ByteWidth = Size;                     // size of the buffer
        BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;        // This buffer will never change
        BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // use in vertex shader
        BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
        BufferDesc.MiscFlags = 0;                        // No other option
        BufferDesc.StructureByteStride = VertexSize;
        
        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
        InitData.pSysMem = Vertices;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        
        Result = State->Device->CreateBuffer(&BufferDesc, &InitData, &State->VertexBufferRectangle);
        if (FAILED(Result)) 
        {
            printf("Failed to create the VertexbufferRectangle!\n");
            return false;
        }
    }
    
    
    
    //
    // Vertex buffer for a circle
    // - @debug, inefficient!
    {
        u32 SliceCount = 32;
        u32 VertexCount = 3 * SliceCount;
        size_t VertexSize = sizeof(v2);
        size_t Size = VertexCount * VertexSize;
        
        v2 *Vertices = static_cast<v2 *>(malloc(Size));
        assert(Vertices);
        
        f32 Theta = Tau32 / (f32)SliceCount;
        f32 CurrAngle = 0.0f;
        u32 VertexIndex = 0;
        for (u32 Index = 0; Index < SliceCount; ++Index)
        {
            Vertices[VertexIndex++] = v2_zero;
            Vertices[VertexIndex++] = V2(Cos(CurrAngle), Sin(CurrAngle));
            CurrAngle += Theta;
            Vertices[VertexIndex++] = V2(Cos(CurrAngle), Sin(CurrAngle));
        }
        assert(VertexIndex == VertexCount);
        
        D3D11_BUFFER_DESC BufferDesc;
        ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
        BufferDesc.ByteWidth = Size;                     // size of the buffer
        BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;        // This buffer will never change
        BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // use in vertex shader
        BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
        BufferDesc.MiscFlags = 0;                        // No other option
        BufferDesc.StructureByteStride = VertexSize;
        
        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
        InitData.pSysMem = Vertices;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        
        Result = State->Device->CreateBuffer(&BufferDesc, &InitData, &State->VertexBufferCircle);
        if (FAILED(Result)) 
        {
            if (Vertices)
            {
                free(Vertices);
            }
            
            printf("Failed to create the VertexbufferRectangle!\n");
            return false;
        }
        
        if (Vertices)
        {
            free(Vertices);
        }
        
        State->VertexCountCircle = VertexCount;
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


void UpdateConstantBuffer(dx_state *State, shader_constants *ShaderConstants)
{
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    ZeroMemory(&MappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    
    State->DeviceContext->Map(State->ShaderConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    memcpy(MappedResource.pData, ShaderConstants, sizeof(shader_constants));
    State->DeviceContext->Unmap(State->ShaderConstantsBuffer, 0);
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
    
    DC->IASetInputLayout(State->ilBasic);
    
    //
    // Clear the depth-/stencil-view and the texture used as render target
    DC->ClearDepthStencilView(State->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    DC->ClearRenderTargetView(State->RenderTargetView, (f32 *)&State->BackgroundColour);
    
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
    DC->ResolveSubresource(State->ResolveTexture, 0,
                           State->RenderTargetTexture, 0,
                           DXGI_FORMAT_B8G8R8A8_UNORM);
    
    
    //
    // Render the resolved texture
    DC->IASetInputLayout(State->ilFullscreen);
    
    //
    // Set the backbuffer (in the swap chain) as the render target
    State->DeviceContext->OMSetDepthStencilState(nullptr, 1);
    State->DeviceContext->OMSetRenderTargets(1, &State->BackbufferView, nullptr);
    
    //
    // Set shaders
    DC->VSSetShader(State->vsFullscreen, nullptr, 0);
    DC->PSSetShader(State->psFullscreen, nullptr, 0);
    
    State->DeviceContext->PSSetShaderResources(0, 1, &State->ResolveShaderView);
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
    State->SwapChain->Present(1, 0); // vertical sync enabled if the first param is 1
}




//
// Draw calls
//

void RenderFilledRectangle(dx_state *State, v2 Po, v2 Size, v4 Colour)
{
    State->ShaderConstants.Colour = Colour;
    State->ShaderConstants.ObjectToWorld = M4Scale(Size.x, Size.y, 1.0f) * M4Translation(Po.x, Po.y, 0.0f);
    UpdateConstantBuffer(State, &State->ShaderConstants);
    
    ID3D11DeviceContext *DC = State->DeviceContext;
    
    size_t Stride = sizeof(v2);
    size_t Offset = 0;
    
    DC->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // @debug
    DC->IASetVertexBuffers(0, 1, &State->VertexBufferRectangle, &Stride, &Offset); 
    DC->Draw(6, 0);
}


void RenderFilledCircle(dx_state *State, v2 Po, f32 Radius, v4 Colour)
{
    State->ShaderConstants.Colour = Colour;
    State->ShaderConstants.ObjectToWorld = M4Scale(Radius, Radius, 1.0f) * M4Translation(Po.x, Po.y, 0.0f);
    UpdateConstantBuffer(State, &State->ShaderConstants);
    
    ID3D11DeviceContext *DC = State->DeviceContext;
    
    size_t Stride = sizeof(v2);
    size_t Offset = 0;
    
    DC->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // @debug
    DC->IASetVertexBuffers(0, 1, &State->VertexBufferCircle, &Stride, &Offset); 
    DC->Draw(State->VertexCountCircle, 0);
}




//
// Process draw_calls
//

void ProcessDrawCalls(dx_state *State, draw_calls *DrawCalls)
{
    BeginRendering(State);
    
    for (u8 *CurrAddress = DrawCalls->Memory;
         CurrAddress < (DrawCalls->Memory + DrawCalls->MemoryUsed);
         )
    {
        draw_call_header *Header = reinterpret_cast<draw_call_header *>(CurrAddress);
        
        switch (Header->Type)
        {
            case DrawCallType_FilledRectangle:
            {
                draw_call_filled_rectangle *DrawCall = reinterpret_cast<draw_call_filled_rectangle *>(CurrAddress);
                DrawCall;
                RenderFilledRectangle(State, DrawCall->P, DrawCall->Size, DrawCall->Colour);
            } break;
            
            case DrawCallType_FilledCircle:
            {
                draw_call_filled_circle *DrawCall = reinterpret_cast<draw_call_filled_circle *>(CurrAddress);
                DrawCall;
                RenderFilledCircle(State, DrawCall->P, DrawCall->Radius, DrawCall->Colour);
            } break;
            
            case DrawCallType_Text:
            {
                draw_call_text *DrawCall = reinterpret_cast<draw_call_text *>(CurrAddress);
                DrawCall;
#if 0
                BeginDraw(&DirectWriteState);
                DrawText(&DirectWriteState, DrawCall->P, DrawCall->Text);
                EndDraw(&DirectWriteState);
#endif
            } break;
            
            default:
            {
                assert(0);
            } break;
        }
        
        CurrAddress += Header->Size;
    }
    
    ClearMemory(DrawCalls);
    
    EndRendering(State);
}