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

#ifndef win32_dx_buffer__h
#define win32_dx_buffer__h

#ifdef DEBUG
#include <stdio.h>
#else
#define printf(...)
#endif

#include <d3d11.h>
#include "types.h"



struct dx_buffer
{
    ID3D11Buffer *Ptr = nullptr;
    size_t Size = 0;
};


b32 CreateDynamicVertexBuffer(ID3D11Device *Device, void *Data, size_t DataSize, size_t ElementSize, dx_buffer *Buffer);
b32 CreateImmutableVertexBuffer(ID3D11Device *Device, void *Data, size_t DataSize, size_t ElementSize, dx_buffer *Buffer);
b32 CreateConstantBuffer(ID3D11Device *Device, void *Data, size_t DataSize, size_t ElementSize, dx_buffer *Buffer);

void UpdateBuffer(ID3D11DeviceContext *DeviceContext, dx_buffer *Buffer, void *Data, size_t DataSize);

void Free(dx_buffer *Buffer);



#endif