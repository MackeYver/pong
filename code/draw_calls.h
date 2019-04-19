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
    u8 *Memory = nullptr;
    size_t MemorySize = 0;
    size_t MemoryUsed = 0;
    
    display_metrics DisplayMetrics;
};

void Init(draw_calls *DrawCalls, size_t Size, display_metrics DisplayMetrics);
void ClearMemory(draw_calls *DrawCalls);
void Shutdown(draw_calls *DrawCalls);




//
// Draw calls
//

void PushFilledRectangle(draw_calls *DrawCalls, v2 P, v2 Size, v4 Colour = v4_one);
void PushTexturedRectangle(draw_calls *DrawCalls, v2 P, v2 Size, texture_index Index, v4 Colour = v4_one);

void PushFilledCircle(draw_calls *DrawCalls, v2 P, f32 Radius, v4 Colour = v4_one);
void PushTexturedCircle(draw_calls *DrawCalls, v2 P, f32 Radius, texture_index Index, v4 Colour = v4_one);

void PushText(draw_calls *DrawCalls, v2 P, wchar_t const *Text);




//
// Draw call structs
//

enum draw_call_type
{
    DrawCallType_FilledRectangle = 0,
    DrawCallType_TexturedRectangle,
    DrawCallType_FilledCircle,
    DrawCallType_TexturedCircle,
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


struct draw_call_textured_rectangle
{
    draw_call_header Header;
    v4 Colour;
    v2 P;
    v2 Size;
    texture_index TextureIndex;
};


struct draw_call_filled_circle
{
    draw_call_header Header;
    v4 Colour;
    v2 P;
    f32 Radius;
};


struct draw_call_textured_circle
{
    draw_call_header Header;
    v4 Colour;
    v2 P;
    f32 Radius;
    texture_index TextureIndex;
};


struct draw_call_text
{
    draw_call_header Header;
    wchar_t *Text;
    v2 P;
};



#endif // Include guard