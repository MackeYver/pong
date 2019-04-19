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
// Init and shutdown, basically memory stuff
//

void Init(draw_calls *DrawCalls, size_t MemorySize, display_metrics DisplayMetrics)
{
    assert(DrawCalls);
    assert(MemorySize > 0);
    
    DrawCalls->Memory = static_cast<u8 *>(calloc(1, MemorySize));
    assert(DrawCalls->Memory);
    
    DrawCalls->MemorySize = MemorySize;
    DrawCalls->MemoryUsed = 0;
    
    DrawCalls->DisplayMetrics = DisplayMetrics;
}


void Shutdown(draw_calls *DrawCalls)
{
    assert(DrawCalls);
    
    if (DrawCalls->Memory)
    {
        free(DrawCalls->Memory);
    }
    
    DrawCalls->Memory = nullptr;
    DrawCalls->MemoryUsed = 0;
    DrawCalls->MemorySize = 0;
}

void ClearMemory(draw_calls *DrawCalls)
{
    assert(DrawCalls);
    DrawCalls->MemoryUsed = 0;
}

u8 *PushMemory(draw_calls *DrawCalls, size_t Size)
{
    assert(DrawCalls);
    assert(DrawCalls->Memory);
    
    u8 *Result = nullptr;
    
    if ((DrawCalls->MemorySize - DrawCalls->MemoryUsed) >= Size)
    {
        Result = (DrawCalls->Memory + DrawCalls->MemoryUsed);
        DrawCalls->MemoryUsed += Size;
    }
    
    return Result;
}



//
// Draw calls
//

void PushFilledRectangle(draw_calls *DrawCalls, v2 P, v2 Size, v4 Colour)
{
    assert(DrawCalls);
    assert(DrawCalls->Memory);
    
    size_t DrawCallSize = sizeof(draw_call_filled_rectangle);
    draw_call_filled_rectangle *DrawCall = reinterpret_cast<draw_call_filled_rectangle *>(PushMemory(DrawCalls, DrawCallSize));
    assert(DrawCall);
    
    DrawCall->Header.Size = DrawCallSize;
    DrawCall->Header.Type = DrawCallType_FilledRectangle;
    
    DrawCall->Colour = Colour;
    DrawCall->P = P;
    DrawCall->Size = Size;
}


void PushTexturedRectangle(draw_calls *DrawCalls, v2 P, v2 Size, texture_index Index, v4 Colour)
{
    assert(DrawCalls);
    assert(DrawCalls->Memory);
    
    size_t DrawCallSize = sizeof(draw_call_textured_rectangle);
    draw_call_textured_rectangle *DrawCall = reinterpret_cast<draw_call_textured_rectangle *>(PushMemory(DrawCalls, DrawCallSize));
    assert(DrawCall);
    
    DrawCall->Header.Size = DrawCallSize;
    DrawCall->Header.Type = DrawCallType_TexturedRectangle;
    
    DrawCall->Colour = Colour;
    DrawCall->P = P;
    DrawCall->Size = Size;
    DrawCall->TextureIndex = Index;
}


void PushFilledCircle(draw_calls *DrawCalls, v2 P, f32 Radius, v4 Colour)
{
    assert(DrawCalls);
    assert(DrawCalls->Memory);
    
    size_t DrawCallSize = sizeof(draw_call_filled_circle);
    draw_call_filled_circle *DrawCall = reinterpret_cast<draw_call_filled_circle *>(PushMemory(DrawCalls, DrawCallSize));
    assert(DrawCall);
    
    DrawCall->Header.Size = DrawCallSize;
    DrawCall->Header.Type = DrawCallType_FilledCircle;
    
    DrawCall->Colour = Colour;
    DrawCall->P = P;
    DrawCall->Radius = Radius;
}


void PushTexturedCircle(draw_calls *DrawCalls, v2 P, f32 Radius, texture_index Index, v4 Colour)
{
    assert(DrawCalls);
    assert(DrawCalls->Memory);
    
    size_t DrawCallSize = sizeof(draw_call_textured_circle);
    draw_call_textured_circle *DrawCall = reinterpret_cast<draw_call_textured_circle *>(PushMemory(DrawCalls, DrawCallSize));
    assert(DrawCall);
    
    DrawCall->Header.Size = DrawCallSize;
    DrawCall->Header.Type = DrawCallType_TexturedCircle;
    
    DrawCall->Colour = Colour;
    DrawCall->P = P;
    DrawCall->Radius = Radius;
    DrawCall->TextureIndex = Index;
}


void PushText(draw_calls *DrawCalls, v2 P, wchar_t const *Text)
{
    assert(DrawCalls);
    assert(DrawCalls->Memory);
    
    size_t DrawCallSize = sizeof(draw_call_text);
    size_t TextSize = (wcslen(Text) + 1) * sizeof(wchar_t);
    size_t TotalSize = DrawCallSize + TextSize;
    
    draw_call_text *DrawCall = reinterpret_cast<draw_call_text *>(PushMemory(DrawCalls, TotalSize));
    assert(DrawCall);
    
    DrawCall->Header.Size = TotalSize;
    DrawCall->Header.Type = DrawCallType_Text;
    
    DrawCall->P = P;
    
    // We store the text in memory, after the draw call struct
    DrawCall->Text = reinterpret_cast<wchar_t *>(DrawCall + DrawCallSize); 
    wcsncpy_s(DrawCall->Text, TextSize, Text, TextSize);
}