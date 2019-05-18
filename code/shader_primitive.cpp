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
#include "shader_primitive.h"
#include "win32_file_io.h"
#include "memory_arena.h"

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

b32 Init(ID3D11Device *Device, shader_primitive *Shader)
{
    //
    // Vertex shader
    //
    
    u8 *Data = nullptr;
    size_t DataSize = 0;
    
    win32_ReadFile("build\\shaders\\primitives_vs.cso", &Data, &DataSize);
    assert(Data);
    assert(DataSize > 0);
    
    HRESULT Result = Device->CreateVertexShader(Data,
                                                DataSize,
                                                nullptr,
                                                &Shader->VertexProgram);
    if (FAILED(Result)) {
        printf("%s: Failed to create vertex shader from file!\n", __FILE__);
        return false;
    }
    
    
    //
    // Input layout
    D3D11_INPUT_ELEMENT_DESC InputElements[] =
    {
        {"Colour"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
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
    
    win32_ReadFile("build\\shaders\\primitives_ps.cso", &Data, &DataSize);
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
    // Constant buffer
    //
    
    {
        size_t Size = sizeof(shader_primitive::constants);
        assert(Size % 16 == 0);
        
        b32 bResult = CreateConstantBuffer(Device, &Shader->Constants, Size, 0, &Shader->ConstantsBuffer);
        if (!bResult)
        {
            printf("%s: Failed to create the shader's ConstantBuffer\n", __FILE__);
            return false;
        }
    }
    
    Shader->Constants.ObjectToWorld = m4_identity;
    Shader->Constants.WorldToClip = m4_identity;
    
    
    
    
    //
    // Vertex buffer
    //
    
    {
        size_t VertexSize = sizeof(shader_primitive::vs_input);
        size_t Size = 1 << 10; // @debug
        b32 bResult = CreateDynamicVertexBuffer(Device, nullptr, Size, VertexSize, &Shader->VertexBuffer);
        if (!bResult)
        {
            printf("%s: Failed to create the vertex buffer for shader_primitives.\n", __FILE__);
            return false;
        }
    }
    
    return true;
}


#define DX_RELEASE(x) Shader->x ? Shader->x->Release() : 0
#define DX_FREE(x) Free(&Shader->x)
void Shutdown(shader_primitive *Shader)
{
    DX_FREE(VertexBuffer);
    DX_FREE(ConstantsBuffer);
    DX_RELEASE(PixelProgram);
    DX_RELEASE(InputLayout);
    DX_RELEASE(VertexProgram);
}




//
// Update constants
//

void UpdateConstants(ID3D11DeviceContext *DeviceContext, shader_primitive *Shader)
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
    memcpy(MappedResource.pData, &Shader->Constants, sizeof(shader_primitive::constants));
    DeviceContext->Unmap(Shader->ConstantsBuffer.Ptr, 0);
}



void Use(ID3D11DeviceContext *DC, shader_primitive *Shader)
{
    DC->IASetInputLayout(Shader->InputLayout);
    
    //
    // Set shaders
    DC->VSSetShader(Shader->VertexProgram, nullptr, 0);
    DC->PSSetShader(Shader->PixelProgram, nullptr, 0);
    
    //
    // Constant buffer
    DC->VSSetConstantBuffers(0, 1, &Shader->ConstantsBuffer.Ptr);
    DC->PSSetConstantBuffers(0, 1, &Shader->ConstantsBuffer.Ptr);
}




//
// Render primitives
//

void DrawPrimitives(ID3D11Device *Device, ID3D11DeviceContext *DC, shader_primitive *Shader, D3D_PRIMITIVE_TOPOLOGY Topology, 
                    memory_arena *Memory, u32 VertexCount)
{
    assert(Topology == D3D_PRIMITIVE_TOPOLOGY_LINELIST || Topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DC->IASetPrimitiveTopology(Topology);
    
    
    //
    // Upload data to the GPU
    if (Shader->VertexBuffer.Size < Memory->Used)
    {
        b32 Result = ResizeBuffer(Device, &Shader->VertexBuffer, Memory->Used);
        assert(Result);
    }
    
    UpdateBuffer(DC, &Shader->VertexBuffer, static_cast<void *>(Memory->Ptr), Memory->Used);
    
    
    //
    // Vertex buffers
    size_t Stride = sizeof(shader_primitive::vs_input);
    size_t Offset = 0;
    DC->IASetVertexBuffers(0, 1, &Shader->VertexBuffer.Ptr, &Stride, &Offset); 
    
    
    //
    // Draw
    DC->Draw(VertexCount, 0);
}

