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



//
// TODO(Marcus): Clean up this code, it works but it's a bit messy!
//

//
// NOTE(Marcus): Supported elements (only the mentioned elements and properties are supported):        
//               - vertex
//                 - Requiered property:
//                   - Positions, named x y z, type float
//
//                 - Optional properties:
//                   - normals, named nx ny nz, type float
//                   - texture coordinates, named s t, type float
//
//               - face
//                 - Required property
//                   - none
// 
//                 - Optional property
//                   - none
//

#ifndef ply_loader__h
#define ply_loader__h

#include "mathematics.h"


struct mesh
{
    v3 *Positions = nullptr;
    v3 *Normals   = nullptr;
    v2 *UVs       = nullptr;
    u16 *Indices  = nullptr;
    
    u32 VertexCount = 0;
    size_t VertexSize;
    u32 FaceCount = 0;
    u32 IndexCount = 0;
};

b32 ParsePLY(u8 *Data, size_t DataSize, mesh *Output);
void Free(mesh *Mesh);



#endif