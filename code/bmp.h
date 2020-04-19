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


#ifndef bmp__h
#define bmp__h

#include "mathematics.h"
#include <windows.h>

//
// Sources: 
// - https://en.wikipedia.org/wiki/BMP_file_format
// - https://www.fileformat.info/format/bmp/egff.htm
//



#pragma pack(push, 1)
struct bitmap_file_header
{
    WORD MagicNumber;
    DWORD FileSize;
    WORD Reserved1;
    WORD Reserved2;
    DWORD DataOffset; // Offset to where the pixel data begins
};
#pragma pack(pop)



#pragma pack(push, 1)
struct dib_header // This is really v3, for now we'll only support this one.
{
    u32 HeaderSize;
    s32 Width;
    s32 Height;
    u16 ColourPLaneCount;
    u16 BitsPerPixel;
    u32 CompressionMethod;
    u32 PixelDataSize;
    s32 PixelPerMeterX;
    s32 PixelPerMeterY;
    u32 PaletteColourCount;
    u32 ImportantColourCount;
}; // size 40
#pragma pack(pop)



struct bmp
{
    dib_header Header = {};
    u8 *Data = nullptr;
    u32 RowSize = 0;
    u32 DataSize = 0;
};

b32 ParseBMP(u8 *Data, u32 DataSize, bmp *Output);
void Free(bmp *Bmp);



#endif
