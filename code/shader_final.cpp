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

#include <d3d11.h>
#include "shader_final.h"
#include "win32_file_io.h"
#include "win32_dx_texture.h"

#ifdef DEBUG
#include <assert.h>
#include <stdio.h>
#else
#define assert(x)
#define printf(...)
#endif




//
// Init
//

b32 Init(ID3D11Device *Device, shader_final *Shader)
{
    //
    // Shaders
    //
    
    //
    // vertex shader
    u8 *Data = nullptr;
    size_t DataSize = 0;
    win32_ReadFile("build\\shaders\\fullscreen_texture_vs.cso", &Data, &DataSize);
    assert(Data);
    assert(DataSize > 0);
    
    // Create shader
    HRESULT Result = Device->CreateVertexShader(Data,
                                                DataSize,
                                                nullptr,
                                                &Shader->VertexProgram);
    if (FAILED(Result)) {
        printf("%s: Failed to create vertex shader from file!\n", __FILE__);
        return false;
    }
    
    // Input layout
    D3D11_INPUT_ELEMENT_DESC InputElements[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    Result = Device->CreateInputLayout(InputElements, 2, Data, DataSize, &Shader->InputLayout);
    
    if (Data)
    {
        free(Data);
    }
    
    //
    // Pixel shader
    win32_ReadFile("build\\shaders\\fullscreen_texture_ps.cso", &Data, &DataSize);
    assert(Data);
    assert(DataSize > 0);
    
    // Create shader
    Result = Device->CreatePixelShader(Data,
                                       DataSize,
                                       nullptr,
                                       &Shader->PixelProgram);
    if (Data)
    {
        free(Data);
        DataSize = 0;
    }
    
    if (FAILED(Result)) {
        printf("%s: Failed to create pixel shader from file!\n", __FILE__);
        return false;
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
        
        Result = Device->CreateSamplerState(&SamplerDesc, &Shader->Sampler);
        if (FAILED(Result))
        {
            printf("%s: Failed to create sampler!\n", __FILE__);
            return false;
        }
    }
    
    
    
    
    //
    // Fullscreen quad, used to render the RenderTargetTexture to the screen
    //
    
    {
        struct pt
        {
            v3 P;
            v2 T;
        };
        
        pt Vertices[] =
        {
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}},
            
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}},
        };
        
        u32 VertexCount = 6;
        size_t VertexSize = sizeof(pt);
        size_t Size = VertexCount * VertexSize;
        b32 bResult = CreateImmutableVertexBuffer(Device, Vertices, Size, VertexSize, &Shader->VertexBuffer);
        if (!bResult)
        {
            printf("%s: Failed to create the buffer with vertex data for the fullscreen-quad.\n", __FILE__);
            return false;
        }
    }
    
    return true;
}




//
// Shutdown
//

#define DX_RELEASE_BUFFER(x) Shader->x.Ptr ? Shader->x.Ptr->Release() : 0
#define DX_RELEASE(x) Shader->x ? Shader->x->Release() : 0
void Shutdown(shader_final *Shader)
{
    DX_RELEASE_BUFFER(VertexBuffer);
    DX_RELEASE(Sampler);
    DX_RELEASE(PixelProgram);
    DX_RELEASE(InputLayout);
    DX_RELEASE(VertexProgram);
}




//
// Ready the pipeline to use the shaders
//

void Use(ID3D11DeviceContext *DC, shader_final *Shader)
{
    DC->IASetInputLayout(Shader->InputLayout);
    DC->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // @debug
    
    //
    // Set shaders
    DC->VSSetShader(Shader->VertexProgram, nullptr, 0);
    DC->PSSetShader(Shader->PixelProgram, nullptr, 0);
    
    //
    // Set resources
    DC->PSSetSamplers(0, 1, &Shader->Sampler);
}




//
// Draw a mesh
//

void DrawTextureToScreen(ID3D11DeviceContext *DC, shader_final *Shader, dx_texture *Texture)
{
    //
    // Vertex buffers
    size_t Stride = sizeof(v3) + sizeof(v2);
    size_t Offset = 0;
    DC->IASetVertexBuffers(0, 1, &Shader->VertexBuffer.Ptr, &Stride, &Offset); 
    
    //
    // Texture
    DC->PSSetShaderResources(0, 1, &Texture->ShaderView);
    
    //
    // Draw
    DC->Draw(6, 0);
}