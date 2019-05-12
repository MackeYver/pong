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

#include "shader_primitive.h"
#include "win32_file_io.h"
#include <d3d11.h>

#ifdef DEBUG
#include <assert.h>
#include <stdio.h>
#else
#define assert(x)
#define printf(...)
#endif



b32 Init(ID3D11Device *Device, shader_primitive *Shader)
{
    //
    // Vertex shader
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
        {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"Colour"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
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
    
    return true;
}


#define DX_RELEASE(x) Shader->x ? Shader->x->Release() : 0
void Shutdown(shader_primitive *Shader)
{
    DX_RELEASE(PixelProgram);
    DX_RELEASE(InputLayout);
    DX_RELEASE(VertexProgram);
}


void Use(ID3D11DeviceContext *DC, shader_primitive *Shader)
{
    DC->IASetInputLayout(Shader->InputLayout);
    DC->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // @debug
    DC->Draw(6, 0);
}