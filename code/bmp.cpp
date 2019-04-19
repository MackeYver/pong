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

#include "bmp.h"
#include "win32_file_io.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif



b32 ParseBMP(u8 *Data, size_t DataSize, bmp *Output)
{
    b32 Result = false;
    
    if (Output && Data && DataSize > 0)
    {
        bitmap_file_header *BFHeader = reinterpret_cast<bitmap_file_header *>(Data);
        if (BFHeader->MagicNumber == 0x4d42)
        {
            dib_header *Header = reinterpret_cast<dib_header *>(Data + sizeof(bitmap_file_header));
            if ((Header->HeaderSize == 40) && (Header->CompressionMethod == 0)) // we only support v3 for now
            {
                memcpy(&Output->Header, Data + sizeof(bitmap_file_header), sizeof(dib_header));
                
                Output->Header.Height = Abs(Output->Header.Height);
                
                Output->RowSize = 4 * (size_t)(((f32)(Header->BitsPerPixel * Output->Header.Width) / 32.0f) + 0.5f);
                Output->DataSize = Output->RowSize * Output->Header.Height;
                assert(Output->DataSize > 0);
                
                Output->Data = static_cast<u8 *>(malloc(Output->DataSize));
                assert(Output->Data);
                memcpy(Output->Data, 
                       Data + sizeof(bitmap_file_header) + sizeof(dib_header), 
                       Output->DataSize);
                
                Result = true;
            }
            else
            {
                printf("%s: File has an invalid header, the size of the header is not 40 bytes\n", __FUNCTION__);
            }
        }
        else
        {
            printf("%s: file is not a bmp\n", __FUNCTION__);
        }
    }
    
    return Result;
}


void Free(bmp *Bmp)
{
    if (Bmp && Bmp->Data)
    {
        free(Bmp->Data);
        Bmp->Data = nullptr;
        Bmp->DataSize = 0;
        Bmp->RowSize = 0;
    }
    
    memset(&Bmp->Header, 0, sizeof(dib_header));
}