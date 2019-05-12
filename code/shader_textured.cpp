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
#include "shader_textured.h"
#include "win32_file_io.h"

#include "win32_dx_mesh.h"
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

b32 Init(ID3D11Device *Device, shader_textured *Shader)
{
    //
    // Vertex shader
    //
    
    u8 *Data = nullptr;
    size_t DataSize = 0;
    
    win32_ReadFile("build\\shaders\\textured_vs.cso", &Data, &DataSize);
    assert(Data);
    assert(DataSize > 0);
    
    HRESULT Result = Device->CreateVertexShader(Data,
                                                DataSize,
                                                nullptr,
                                                &Shader->VertexProgram);
    if (FAILED(Result)) {
        printf("%s: Failed to create vertex shader!\n", __FILE__);
        return false;
    }
    
    
    
    
    //
    // Input layout
    //
    
    D3D11_INPUT_ELEMENT_DESC InputElements[] =
    {
        {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT,    1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    
    Result = Device->CreateInputLayout(InputElements, 2, Data, DataSize, &Shader->InputLayout);
    if (FAILED(Result))
    {
        printf("%s: Failed to create input layout!\n", __FILE__);
        return false;
    }
    
    if (Data)
    {
        free(Data);
    }
    
    
    
    
    //
    // Pixel shader
    //
    
    win32_ReadFile("build\\shaders\\textured_ps.cso", &Data, &DataSize);
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
    // Constant buffer
    //
    
    {
        size_t Size = sizeof(shader_textured::constants);
        assert(Size % 16 == 0);
        
        b32 bResult = CreateConstantBuffer(Device, &Shader->Constants, Size, 0, &Shader->ConstantsBuffer);
        if (!bResult)
        {
            printf("Failed to create the shader constantbuffer\n");
            return false;
        }
    }
    
    Shader->Constants.ObjectToWorld = m4_identity;
    Shader->Constants.WorldToClip = m4_identity;
    Shader->Constants.Colour = v4_one;
    
    return true;
}




//
// Shutdown
//

#define DX_RELEASE_BUFFER(x) Shader->x.Ptr ? Shader->x.Ptr->Release() : 0
#define DX_RELEASE(x) Shader->x ? Shader->x->Release() : 0
void Shutdown(shader_textured *Shader)
{
    DX_RELEASE_BUFFER(ConstantsBuffer);
    DX_RELEASE(Sampler);
    DX_RELEASE(PixelProgram);
    DX_RELEASE(InputLayout);
    DX_RELEASE(VertexProgram);
}




//
// Update constants
//

void UpdateConstants(ID3D11DeviceContext *DeviceContext, shader_textured *Shader)
{
#ifdef DEBUG
    assert(Shader);
    assert(Shader->ConstantsBuffer.Ptr);
    
    D3D11_BUFFER_DESC Desc;
    Shader->ConstantsBuffer.Ptr->GetDesc(&Desc);
    
    assert(Desc.ByteWidth == Shader->ConstantsBuffer.Size);
    assert(Desc.Usage != D3D11_USAGE_IMMUTABLE);
    assert(Desc.CPUAccessFlags == D3D11_CPU_ACCESS_WRITE);
#endif
    
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    ZeroMemory(&MappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    
    DeviceContext->Map(Shader->ConstantsBuffer.Ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    memcpy(MappedResource.pData, &Shader->Constants, sizeof(shader_textured::constants));
    DeviceContext->Unmap(Shader->ConstantsBuffer.Ptr, 0);
}




//
// Ready the pipeline to use the shaders
//

void Use(ID3D11DeviceContext *DC, shader_textured *Shader)
{
    DC->IASetInputLayout(Shader->InputLayout);
    DC->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // @debug
    
    //
    // Set shaders
    DC->VSSetShader(Shader->VertexProgram, nullptr, 0);
    DC->PSSetShader(Shader->PixelProgram, nullptr, 0);
    
    //
    // Constant buffer
    DC->VSSetConstantBuffers(0, 1, &Shader->ConstantsBuffer.Ptr);
    DC->PSSetConstantBuffers(0, 1, &Shader->ConstantsBuffer.Ptr);
    
    //
    // Set resources
    DC->PSSetSamplers(0, 1, &Shader->Sampler);
}




//
// Draw a mesh
//

void DrawMesh(ID3D11DeviceContext *DC, shader_textured *Shader, dx_mesh *Mesh, dx_texture *Texture)
{
    //
    // Vertex buffers
    size_t Stride = sizeof(v3);
    size_t Offset = 0;
    DC->IASetVertexBuffers(0, 1, &Mesh->Positions, &Stride, &Offset); 
    
    Stride = sizeof(v2);
    DC->IASetVertexBuffers(1, 1, &Mesh->UVs, &Stride, &Offset); 
    
    //
    // Texture
    DC->PSSetShaderResources(0, 1, &Texture->ShaderView);
    
    //
    // Draw
    DC->Draw(Mesh->VertexCount, 0);
}