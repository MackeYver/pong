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

#ifndef renderer__h
#define renderer__h

#include "mathematics.h"


//
// Renderer
//

struct display_metrics
{
    u32 WindowWidth;
    u32 WindowHeight;
    
    u32 ScreenWidth;
    u32 ScreenHeight;
};


struct renderer
{
    u8 *Memory = nullptr;
    size_t MemorySize = 0;
    size_t MemoryUsed = 0;
    
    display_metrics DisplayMetrics;
};

void Init(renderer *Renderer, size_t Size, display_metrics DisplayMetrics);
void ClearMemory(renderer *Renderer);
void Shutdown(renderer *Renderer);




//
// Draw call structs
//

enum draw_call_type
{
    DrawCallType_FilledRectangle = 0,
    DrawCallType_Text,
    
    DrawCallType_Count,
};

struct draw_call_header
{
    size_t Size;
    draw_call_type Type;
};

struct draw_call_filled_rectangle
{
    draw_call_header Header;
    v4 Colour;
    v2 P;
    v2 Size;
};

struct draw_call_text
{
    draw_call_header Header;
    wchar_t *Text;
    v2 P;
};

void PushFilledRectangle(renderer *Renderer, v2 P, v2 Size, v4 Colour = v4_one);
void PushText(renderer *Renderer, v2 P, wchar_t const *Text); 



#endif // Include guard