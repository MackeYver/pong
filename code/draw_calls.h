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

#ifndef draw_calls__h
#define draw_calls__h

#include "mathematics.h"
#include "resources.h"
#include "memory_arena.h"


//
// Draw call management
//

struct display_metrics
{
    u32 WindowWidth;
    u32 WindowHeight;
    
    u32 ScreenWidth;
    u32 ScreenHeight;
};

struct draw_calls
{
    memory_arena Memory;
    
    memory_arena PrimitiveLinesMemory;
    u32 LineCount;
    
    memory_arena PrimitiveTrianglesMemory;
    u32 TriangleCount;
    
    display_metrics DisplayMetrics;
};

void Init(draw_calls *DrawCalls, size_t Size, display_metrics DisplayMetrics);
void Clear(draw_calls *DrawCalls);
void Shutdown(draw_calls *DrawCalls);




//
// Draw calls
//

void PushPrimitiveLine(draw_calls *DrawCalls, v3 P0, v3 P1, v4 Colour);

void PushPrimitiveTriangleOutline(draw_calls *DrawCalls, v3 P0, v3 P1, v3 P2, v4 Colour);
void PushPrimitiveTriangleFilled(draw_calls *DrawCalls, v3 P0, v3 P1, v3 P2, v4 Colour);

void PushPrimitiveRectangleOutline(draw_calls *DrawCalls, v3 P0, v3 P1, v4 Colour);
void PushPrimitiveRectangleFilled(draw_calls *DrawCalls, v3 P0, v3 P1, v4 Colour);

void PushText(draw_calls *DrawCalls, v2 P, wchar_t const *Text);
void PushTexturedMesh(draw_calls *DrawCalls, v3 P, mesh_index MIndex, texture_index TIndex, v2 Size = v2_one, v4 Colour = v4_one);




//
// Draw call structs
//

enum draw_call_type
{
    DrawCallType_Text,
    DrawCallType_TexturedMesh,
    
    DrawCallType_Count,
};


struct draw_call_header
{
    size_t Size;
    draw_call_type Type;
};


struct draw_call_text
{
    draw_call_header Header;
    wchar_t *Text;
    v2 P;
    f32 Rotation;
};


struct draw_call_textured_mesh
{
    draw_call_header Header;
    m4 ObjectToWorld;
    v4 Colour;
    mesh_index MeshIndex;
    texture_index TextureIndex;
};



#endif // Include guard