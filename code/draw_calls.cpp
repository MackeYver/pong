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

#include "draw_calls.h"
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif




//
// Init and shutdown
//

void Init(draw_calls *DrawCalls, size_t MemorySize, display_metrics DisplayMetrics)
{
    assert(DrawCalls);
    assert(MemorySize > 0);
    
    DrawCalls->DisplayMetrics = DisplayMetrics;
    
    Init(&DrawCalls->Memory, MemorySize);
    
    Init(&DrawCalls->PrimitiveLinesMemory, 1 << 10); // @debug
    Init(&DrawCalls->PrimitiveTrianglesMemory, 1 << 10); // @debug
}


void Shutdown(draw_calls *DrawCalls)
{
    assert(DrawCalls);
    Free(&DrawCalls->Memory);
    Free(&DrawCalls->PrimitiveLinesMemory);
    Free(&DrawCalls->PrimitiveTrianglesMemory);
}


void ClearMemory(draw_calls *DrawCalls)
{
    assert(DrawCalls);
    Clear(&DrawCalls->Memory);
    
    Clear(&DrawCalls->PrimitiveLinesMemory);
    DrawCalls->LineCount = 0;
    
    Clear(&DrawCalls->PrimitiveTrianglesMemory);
    DrawCalls->TriangleCount = 0;
}




//
// Draw calls, primitives
//

struct vertex_PC
{
    v4 C;
    v3 P;
};

void PushPrimitiveLine(draw_calls *DrawCalls, v3 P0, v3 P1, v4 Colour)
{
    size_t Size = 2 * sizeof(vertex_PC);
    if (RemainingSize(&DrawCalls->PrimitiveLinesMemory) < Size)
    {
        b32 Result = Resize(&DrawCalls->PrimitiveLinesMemory, 2 * DrawCalls->PrimitiveLinesMemory.Size);
        assert(Result);
    }
    
    vertex_PC *Vertices = reinterpret_cast<vertex_PC *>(Push(&DrawCalls->PrimitiveLinesMemory, Size));
    Vertices[0].P = P0;
    Vertices[0].C = Colour;
    Vertices[1].P = P1;
    Vertices[1].C = Colour;
    
    ++DrawCalls->LineCount;
}


void PushPrimitiveRectangleOutline(draw_calls *DrawCalls, v3 P0, v3 P1, v4 Colour)
{
    v3 P01 = V3(P1.x, P0.y, 0.5f * (P0.z + P1.z));
    v3 P10 = V3(P0.x, P1.y, 0.5f * (P0.z + P1.z));
    
    PushPrimitiveLine(DrawCalls, P0, P01, Colour);
    PushPrimitiveLine(DrawCalls, P01, P1, Colour);
    PushPrimitiveLine(DrawCalls, P1, P10, Colour);
    PushPrimitiveLine(DrawCalls, P10, P0, Colour);
}


void PushPrimitiveTriangleOutline(draw_calls *DrawCalls, v3 P0, v3 P1, v3 P2, v4 Colour)
{
    PushPrimitiveLine(DrawCalls, P0, P1, Colour);
    PushPrimitiveLine(DrawCalls, P1, P2, Colour);
    PushPrimitiveLine(DrawCalls, P2, P0, Colour);
}


void PushPrimitiveTriangleFilled(draw_calls *DrawCalls, v3 P0, v3 P1, v3 P2, v4 Colour)
{
    size_t Size = 3 * sizeof(vertex_PC);
    if (RemainingSize(&DrawCalls->PrimitiveTrianglesMemory) < Size)
    {
        b32 Result = Resize(&DrawCalls->PrimitiveTrianglesMemory, 2 * DrawCalls->PrimitiveTrianglesMemory.Size);
        assert(Result);
    }
    
    vertex_PC *Vertices = reinterpret_cast<vertex_PC *>(Push(&DrawCalls->PrimitiveTrianglesMemory, Size));
    Vertices[0].P = P0;
    Vertices[0].C = Colour;
    Vertices[1].P = P1;
    Vertices[1].C = Colour;
    Vertices[2].P = P2;
    Vertices[2].C = Colour;
    
    DrawCalls->TriangleCount += 3;
}


void PushPrimitiveRectangleFilled(draw_calls *DrawCalls, v3 P0, v3 P1, v4 Colour)
{
    v3 P01 = V3(P1.x, P0.y, 0.5f * (P0.z + P1.z));
    v3 P10 = V3(P0.x, P1.y, 0.5f * (P0.z + P1.z));
    
    PushPrimitiveTriangleFilled(DrawCalls, P0, P01, P1, Colour);
    PushPrimitiveTriangleFilled(DrawCalls, P0, P1, P10, Colour);
}




//
// Draw calls
//

void PushText(draw_calls *DrawCalls, v2 P, wchar_t const *Text)
{
    size_t DrawCallSize = sizeof(draw_call_text);
    size_t TextSize = (wcslen(Text) + 1) * sizeof(wchar_t);
    size_t TotalSize = DrawCallSize + TextSize;
    
    draw_call_text *DrawCall = reinterpret_cast<draw_call_text *>(Push(&DrawCalls->Memory, TotalSize));
    assert(DrawCall);
    
    DrawCall->Header.Size = TotalSize;
    DrawCall->Header.Type = DrawCallType_Text;
    
    DrawCall->P = P;
    
    // We store the text in memory, after the draw call struct
    DrawCall->Text = reinterpret_cast<wchar_t *>(DrawCall + DrawCallSize); 
    wcsncpy_s(DrawCall->Text, TextSize, Text, TextSize);
}


void PushTexturedMesh(draw_calls *DrawCalls, v3 P, mesh_index MeshIndex, texture_index TextureIndex, v2 Size, v4 Colour)
{
    assert(DrawCalls);
    size_t DrawCallSize = sizeof(draw_call_textured_mesh);
    
    draw_call_textured_mesh *DrawCall = reinterpret_cast<draw_call_textured_mesh *>(Push(&DrawCalls->Memory, DrawCallSize));
    assert(DrawCall);
    
    DrawCall->Header.Size = DrawCallSize;
    DrawCall->Header.Type = DrawCallType_TexturedMesh;
    
    DrawCall->ObjectToWorld = M4Scale(Size.x, Size.y, 1.0f) * M4Translation(P.x, P.y, P.z);
    DrawCall->MeshIndex = MeshIndex;
    DrawCall->TextureIndex = TextureIndex;
    DrawCall->Colour = Colour;
}