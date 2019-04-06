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

#include "renderer.h"
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

void Init(renderer *Renderer, size_t MemorySize, display_metrics DisplayMetrics)
{
    assert(Renderer);
    assert(MemorySize > 0);
    
    Renderer->Memory = static_cast<u8 *>(calloc(1, MemorySize));
    assert(Renderer->Memory);
    
    Renderer->MemorySize = MemorySize;
    Renderer->MemoryUsed = 0;
    
    Renderer->DisplayMetrics = DisplayMetrics;
}


void Shutdown(renderer *Renderer)
{
    assert(Renderer);
    
    if (Renderer->Memory)
    {
        free(Renderer->Memory);
    }
    
    Renderer->Memory = nullptr;
    Renderer->MemoryUsed = 0;
    Renderer->MemorySize = 0;
}

void ClearMemory(renderer *Renderer)
{
    assert(Renderer);
    Renderer->MemoryUsed = 0;
}

u8 *PushMemory(renderer *Renderer, size_t Size)
{
    assert(Renderer);
    assert(Renderer->Memory);
    
    u8 *Result = nullptr;
    
    if ((Renderer->MemorySize - Renderer->MemoryUsed) >= Size)
    {
        Result = (Renderer->Memory + Renderer->MemoryUsed);
        Renderer->MemoryUsed += Size;
    }
    
    return Result;
}



//
// Draw calls
//

void PushFilledRectangle(renderer *Renderer, v2 P, v2 Size, v4 Colour)
{
    assert(Renderer);
    assert(Renderer->Memory);
    
    size_t DrawCallSize = sizeof(draw_call_filled_rectangle);
    draw_call_filled_rectangle *DrawCall = reinterpret_cast<draw_call_filled_rectangle *>(PushMemory(Renderer, DrawCallSize));
    assert(DrawCall);
    
    DrawCall->Header.Size = DrawCallSize;
    DrawCall->Header.Type = DrawCallType_FilledRectangle;
    
    DrawCall->Colour = Colour;
    DrawCall->P = P;
    DrawCall->Size = Size;
}

void PushText(renderer *Renderer, v2 P, wchar_t const *Text)
{
    assert(Renderer);
    assert(Renderer->Memory);
    
    size_t DrawCallSize = sizeof(draw_call_text);
    size_t TextSize = (wcslen(Text) + 1) * sizeof(wchar_t);
    size_t TotalSize = DrawCallSize + TextSize;
    
    draw_call_text *DrawCall = reinterpret_cast<draw_call_text *>(PushMemory(Renderer, TotalSize));
    assert(DrawCall);
    
    DrawCall->Header.Size = TotalSize;
    DrawCall->Header.Type = DrawCallType_Text;
    
    DrawCall->P = P;
    
    // We store the text in memory, after the draw call struct
    DrawCall->Text = reinterpret_cast<wchar_t *>(DrawCall + DrawCallSize); 
    wcsncpy_s(DrawCall->Text, TextSize, Text, TextSize);
}