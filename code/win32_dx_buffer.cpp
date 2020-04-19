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

#include "win32_dx_buffer.h"

#ifdef DEBUG
#include <assert.h>
#include <stdio.h>
#else
#define printf(...)
#endif



//
// Create buffer
//

b32 CreateBuffer(ID3D11Device *Device, 
                 void *Data, u32 DataSize, u32 ElementSize, 
                 D3D11_USAGE Usage,
                 D3D11_BIND_FLAG BindFlags,
                 unsigned int CPUAccess,
                 dx_buffer *Buffer)
{
    b32 Result = true;
    
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    
    BufferDesc.ByteWidth = DataSize;       
    BufferDesc.Usage = Usage;              
    BufferDesc.BindFlags = BindFlags;      
    BufferDesc.CPUAccessFlags = static_cast<D3D11_CPU_ACCESS_FLAG>(CPUAccess);
    BufferDesc.MiscFlags = 0;                    
    BufferDesc.StructureByteStride = ElementSize;
    
    HRESULT HResult;
    if (Data && DataSize > 0)
    {
        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
        
        InitData.pSysMem = Data;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        
        HResult = Device->CreateBuffer(&BufferDesc, &InitData, &Buffer->Ptr);
    }
    else
    {
        HResult = Device->CreateBuffer(&BufferDesc, nullptr, &Buffer->Ptr);
    }
    if (FAILED(HResult)) 
    {
        printf("%s: failed to create a buffer.\n", __FILE__);
        Result = false;
    }
    
    Buffer->Size = DataSize;
    
    return Result;
}


b32 CreateImmutableVertexBuffer(ID3D11Device *Device, void *Data, u32 DataSize, u32 ElementSize, dx_buffer *Buffer)
{
    b32 Result = false;
    Result = CreateBuffer(Device, Data, DataSize, ElementSize, 
                          D3D11_USAGE_IMMUTABLE, 
                          D3D11_BIND_VERTEX_BUFFER, 
                          0, // No CPU access to this buffer!
                          Buffer);
    return Result;
}


b32 CreateDynamicVertexBuffer(ID3D11Device *Device, void *Data, u32 DataSize, u32 ElementSize, dx_buffer *Buffer)
{
    b32 Result = false;
    Result = CreateBuffer(Device, Data, DataSize, ElementSize, 
                          D3D11_USAGE_DYNAMIC, 
                          D3D11_BIND_VERTEX_BUFFER, 
                          D3D11_CPU_ACCESS_WRITE,
                          Buffer);
    return Result;
}


b32 CreateConstantBuffer(ID3D11Device *Device, void *Data, u32 DataSize, u32 ElementSize, dx_buffer *Buffer)
{
    b32 Result = false;
    Result = CreateBuffer(Device, Data, DataSize, ElementSize, 
                          D3D11_USAGE_DYNAMIC, 
                          D3D11_BIND_CONSTANT_BUFFER, 
                          D3D11_CPU_ACCESS_WRITE,
                          Buffer);
    return Result;
}




//
// Update
//

void UpdateBuffer(ID3D11DeviceContext *DeviceContext, dx_buffer *Buffer, void *Data, u32 DataSize)
{
#ifdef DEBUG
    assert(Buffer);
    assert(Buffer->Ptr);
    
    D3D11_BUFFER_DESC Desc;
    Buffer->Ptr->GetDesc(&Desc);
    
    assert(Desc.ByteWidth == Buffer->Size);
    assert(Desc.Usage != D3D11_USAGE_IMMUTABLE);
    assert(Desc.CPUAccessFlags == D3D11_CPU_ACCESS_WRITE);
#endif
    
    if (DataSize > Buffer->Size)
    {
        printf("%s: Uploaded data is larger than the buffer...\n", __FILE__);
    }
    
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    ZeroMemory(&MappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    
    DeviceContext->Map(Buffer->Ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    memcpy(MappedResource.pData, Data, DataSize);
    DeviceContext->Unmap(Buffer->Ptr, 0);
}


b32 ResizeBuffer(ID3D11Device *Device, dx_buffer *Buffer, u32 NewSize)
{
    D3D11_BUFFER_DESC Desc;
    Buffer->Ptr->GetDesc(&Desc);
    
    Free(Buffer);
    
    b32 Result = CreateDynamicVertexBuffer(Device, nullptr, NewSize, Desc.StructureByteStride, Buffer);
    if (!Result)
    {
        printf("%s: Failed to create the vertex buffer for shader_primitives.\n", __FILE__);
    }
    
    Buffer->Size = NewSize;
    
    return Result;
}




//
// Free
//

void Free(dx_buffer *Buffer)
{
    Buffer->Ptr ? Buffer->Ptr->Release() : 0;
};
