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
    u32 Width;
    u32 Height;
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

#if 0
struct dib_header_v4
{
    u32 HeaderSize;
    u32 Width;
    u32 Height;
    u16 ColourPLaneCount;
    u16 BitsPerPixel;
    u32 CompressionMethod;
    u32 PixelDataSize;
    s32 PixelPerMeterX;
    s32 PixelPerMeterY;
    u32 PaletteColourCount;
    u32 ImportantColourCount;
    
    
	u32 RedMask;       /* Mask identifying bits of red component */
	u32 GreenMask;     /* Mask identifying bits of green component */
	u32 BlueMask;      /* Mask identifying bits of blue component */
	u32 AlphaMask;     /* Mask identifying bits of alpha component */
	u32 CSType;        /* Color space type */
	u32 RedX;          /* X coordinate of red endpoint */
	u32 RedY;          /* Y coordinate of red endpoint */
	u32 RedZ;          /* Z coordinate of red endpoint */
	u32 GreenX;        /* X coordinate of green endpoint */
	u32 GreenY;        /* Y coordinate of green endpoint */
	u32 GreenZ;        /* Z coordinate of green endpoint */
	u32 BlueX;         /* X coordinate of blue endpoint */
	u32 BlueY;         /* Y coordinate of blue endpoint */
	u32 BlueZ;         /* Z coordinate of blue endpoint */
	u32 GammaRed;      /* Gamma red coordinate scale value */
	u32 GammaGreen;    /* Gamma green coordinate scale value */
	u32 GammaBlue;     /* Gamma blue coordinate scale value */
}; // size 108


struct dib_header_v5
{
    u32 HeaderSize;
    u32 Width;
    u32 Height;
    u16 ColourPLaneCount;
    u16 BitsPerPixel;
    u32 CompressionMethod;
    u32 ImageSize; // 0 if Compression == 0?
    s32 PixelPerMeterX;
    s32 PixelPerMeterY;
    u32 PaletteColourCount;
    u32 ImportantColourCount;
    
	u32 RedMask;       /* Mask identifying bits of red component */
	u32 GreenMask;     /* Mask identifying bits of green component */
	u32 BlueMask;      /* Mask identifying bits of blue component */
	u32 AlphaMask;     /* Mask identifying bits of alpha component */
	u32 CSType;        /* Color space type */
	u32 RedX;          /* X coordinate of red endpoint */
	u32 RedY;          /* Y coordinate of red endpoint */
	u32 RedZ;          /* Z coordinate of red endpoint */
	u32 GreenX;        /* X coordinate of green endpoint */
	u32 GreenY;        /* Y coordinate of green endpoint */
	u32 GreenZ;        /* Z coordinate of green endpoint */
	u32 BlueX;         /* X coordinate of blue endpoint */
	u32 BlueY;         /* Y coordinate of blue endpoint */
	u32 BlueZ;         /* Z coordinate of blue endpoint */
	u32 GammaRed;      /* Gamma red coordinate scale value */
	u32 GammaGreen;    /* Gamma green coordinate scale value */
	u32 GammaBlue;     /* Gamma blue coordinate scale value */
    u32 Intent;
    u32 ProfileData;
    u32 ProfileSize;
    u32 V5Reserved;
}; // size 123
#endif


struct bmp
{
    dib_header Header = {};
    u8 *Data = nullptr;
    size_t RowSize = 0;
    size_t DataSize = 0;
};


b32 Load(char const *PathAndFilename, bmp *Output);
void Free(bmp *Bmp);



#endif