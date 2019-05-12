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

#include "win32_dx_mesh.h"
#include "win32_dx.h"
#include "mesh.h"



b32 CreateBuffer(dx_state *State, void *Data, size_t DataSize, size_t ElementSize, ID3D11Buffer **Output)
{
    b32 Result = true;
    
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    
    BufferDesc.ByteWidth = DataSize;                 // size of the buffer
    BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;        // This buffer will never change
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // use in vertex shader
    BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
    BufferDesc.MiscFlags = 0;                        // No other option
    BufferDesc.StructureByteStride = ElementSize;
    
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
    
    InitData.pSysMem = Data;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    
    HRESULT HResult = State->Device->CreateBuffer(&BufferDesc, &InitData, Output);
    if (FAILED(HResult)) 
    {
        Result = false;
    }
    
    return Result;
}


s32 CreateMesh(void *DXState, mesh *Mesh)
{
    dx_state *State = reinterpret_cast<dx_state *>(DXState); // CRAZY!
    mesh_index Result = -10;
    
    if (Mesh)
    {
        dx_mesh DXMesh;
        
        // TODO(Marcus): We're assuming that it is triangles, but we don't know. Fix this!
        DXMesh.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        
        assert(Mesh->Positions);
        size_t Size = sizeof(v3) * Mesh->VertexCount;
        if (!CreateBuffer(State, Mesh->Positions, Size, sizeof(v3), &DXMesh.Positions))
        {
            printf("Failed to create the buffer with the positions!\n");
            Result = -1;
        }
        else
        {
            DXMesh.VertexCount = Mesh->VertexCount;
        }
        
        if (Mesh->Normals && Result != -1)
        {
            Size = sizeof(v3) * Mesh->VertexCount;
            if (!CreateBuffer(State, Mesh->Normals, Size, sizeof(v3), &DXMesh.Normals))
            {
                printf("Failed to create the buffer with the normals!\n");
                Result = -1;
            }
        }
        
        if (Mesh->UVs && Result != -1)
        {
            Size = sizeof(v2) * Mesh->VertexCount;
            if (!CreateBuffer(State, Mesh->UVs, Size, sizeof(v2), &DXMesh.UVs))
            {
                printf("Failed to create the buffer with the texture coordinates!\n");
                Result = -1;
            }
        }
        
        if (Mesh->Indices && Result != -1)
        {
            Size = sizeof(u16) * Mesh->IndexCount;
            if (!CreateBuffer(State, Mesh->Indices, Size, sizeof(u16), &DXMesh.Indices))
            {
                printf("Failed to create the buffer with the indices!\n");
                Result = -1;
            }
            DXMesh.IndexCount = Mesh->IndexCount;
        }
        
        if (Result != 1)
        {
            State->Meshes.push_back(DXMesh);
            Result = State->Meshes.size() - 1;
        }
    }
    
    return Result;
}


void Free(dx_mesh *Mesh)
{
    if (Mesh)
    {
        Mesh->Positions ? Mesh->Positions->Release() : 0;
        Mesh->Normals   ? Mesh->Normals->Release()   : 0;
        Mesh->UVs       ? Mesh->UVs->Release()       : 0;
        Mesh->Indices   ? Mesh->Indices->Release()   : 0;
        
        Mesh->Positions = nullptr;
        Mesh->Normals = nullptr;
        Mesh->UVs = nullptr;
        Mesh->Indices = nullptr;
        
        Mesh->VertexCount = 0;
        Mesh->IndexCount = 0;
    }
}