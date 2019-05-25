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


void PushLine(draw_calls *DrawCalls, v3 P0, v3 P1, v4 Colour)
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


void PushTriangleOutline(draw_calls *DrawCalls, v3 P0, v3 P1, v3 P2, v4 Colour)
{
    PushLine(DrawCalls, P0, P1, Colour);
    PushLine(DrawCalls, P1, P2, Colour);
    PushLine(DrawCalls, P2, P0, Colour);
}


void PushTriangleFilled(draw_calls *DrawCalls, v3 P0, v3 P1, v3 P2, v4 Colour)
{
    size_t Size = 3 * sizeof(vertex_PC);
    if (RemainingSize(&DrawCalls->PrimitiveTrianglesMemory) < Size)
    {
        b32 Result = Resize(&DrawCalls->PrimitiveTrianglesMemory, 2 * DrawCalls->PrimitiveTrianglesMemory.Size);
        assert(Result);
    }
    
    vertex_PC *Vertices = reinterpret_cast<vertex_PC *>(Push(&DrawCalls->PrimitiveTrianglesMemory, Size));
    assert(Vertices);
    
    Vertices[0].P = P0;
    Vertices[0].C = Colour;
    Vertices[1].P = P1;
    Vertices[1].C = Colour;
    Vertices[2].P = P2;
    Vertices[2].C = Colour;
    
    ++DrawCalls->TriangleCount;
}


void PushRectangleOutline(draw_calls *DrawCalls, v3 P0, v3 P1, v4 Colour)
{
    v3 P01 = V3(P1.x, P0.y, 0.5f * (P0.z + P1.z));
    v3 P10 = V3(P0.x, P1.y, 0.5f * (P0.z + P1.z));
    
    PushLine(DrawCalls, P0, P01, Colour);
    PushLine(DrawCalls, P01, P1, Colour);
    PushLine(DrawCalls, P1, P10, Colour);
    PushLine(DrawCalls, P10, P0, Colour);
}


void PushRectangleFilled(draw_calls *DrawCalls, v3 P0, v3 P1, v4 Colour)
{
    v3 P01 = V3(P1.x, P0.y, 0.5f * (P0.z + P1.z));
    v3 P10 = V3(P0.x, P1.y, 0.5f * (P0.z + P1.z));
    
    PushTriangleFilled(DrawCalls, P0, P01, P1, Colour);
    PushTriangleFilled(DrawCalls, P0, P1, P10, Colour);
}


void PushRectangleFilled(draw_calls *DrawCalls, v3 P, v2 Size, v4 Colour)
{
    v3 HalfSize = 0.5f * V3(Size, 0.0f);
    v3 P0 = P - HalfSize;
    v3 P1 = P + HalfSize;
    
    PushRectangleFilled(DrawCalls, P0, P1, Colour);
}


void PushCircleOutline(draw_calls *DrawCalls, v3 P, f32 Radius, v4 Colour, u32 SliceCount)
{
    f32 Theta = Tau32 / (f32)SliceCount;
    f32 Angle = 0.0f;
    
    v3 P0 = P + Radius * V3(Sin(Angle), Cos(Angle), 0.0f);
    
    for (u32 Index = 0; Index < SliceCount; ++Index)
    {
        Angle += Theta;
        v3 P1 = P + Radius * V3(Cos(Angle), Sin(Angle), 0.0f);
        
        PushLine(DrawCalls, P0, P1, Colour);
        
        P0 = P1;
    }
}


void PushCircleFilled(draw_calls *DrawCalls, v3 P, f32 Radius, v4 Colour, u32 SliceCount)
{
    f32 Theta = Tau32 / (f32)SliceCount;
    f32 Angle = 0.0f;
    
    v3 P0 = P + Radius * V3(Sin(Angle), Cos(Angle), 0.0f);
    
    for (u32 Index = 0; Index <= SliceCount; ++Index)
    {
        Angle += Theta;
        v3 P1 = P + Radius * V3(Cos(Angle), Sin(Angle), 0.0f);
        
        PushTriangleFilled(DrawCalls, P, P0, P1, Colour);
        
        P0 = P1;
    }
}




//
// Draw calls
//

void PushText(draw_calls *DrawCalls, v2 P, wchar_t const *Text, size_index SizeIndex, colour_index ColourIndex)
{
    size_t DrawCallSize = sizeof(draw_call_text);
    size_t TextSize = (wcslen(Text) + 1) * sizeof(wchar_t);
    size_t TotalSize = DrawCallSize + TextSize;
    
    draw_call_text *DrawCall = reinterpret_cast<draw_call_text *>(Push(&DrawCalls->Memory, TotalSize));
    assert(DrawCall);
    
    DrawCall->Header.Size = TotalSize;
    DrawCall->Header.Type = DrawCallType_Text;
    
    DrawCall->P = P;
    DrawCall->ColourIndex = ColourIndex;
    DrawCall->SizeIndex = SizeIndex;
    
    // We store the text in memory, after the draw call struct
    //DrawCall->Text = reinterpret_cast<wchar_t *>(DrawCall + DrawCallSize); 
    DrawCall->Text = reinterpret_cast<wchar_t *>(reinterpret_cast<size_t>(DrawCall) + DrawCallSize); 
    wcsncpy_s(DrawCall->Text, TextSize, Text, TextSize);
}


void PushShadowedText(draw_calls *DrawCalls, v2 P, wchar_t const *Text, size_index Size, colour_index SC, colour_index TC)
{
    v2 Offset = V2(3.0f, 3.0f);
    PushText(DrawCalls, P - Offset, Text, Size, SC);
    PushText(DrawCalls,          P, Text, Size, TC);
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