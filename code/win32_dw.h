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


#ifndef win32_dw__h
#define win32_dw__h

#include <dwrite.h> // DirectWrite (Dwrite.lib)
#include <d2d1_1.h> // Direct2D    (D2d1_1.lib)

#include "mathematics.h"

struct dx_state; // found in win32_dx.h



struct dw_state
{
    ID2D1Device *Device = nullptr;
    ID2D1DeviceContext *DeviceContext = nullptr;
    ID2D1Bitmap1 *RenderTarget = nullptr;
    ID2D1SolidColorBrush *Brushes[2] = {};
    IDWriteTextFormat *TextFormats[3] = {};
};

b32 Init(dw_state *State, dx_state *DXState);
void Shutdown(dw_state *State);

void BeginDraw(dw_state *State);
HRESULT EndDraw(dw_state *State);

void DrawText(dw_state *State, v2 P, wchar_t const *String, size_index Size = SI_Medium, colour_index Colour = CI_White);


#endif