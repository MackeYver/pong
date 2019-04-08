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

#ifndef wav__h
#define wav__h

#include "types.h"


//
// Currently this only supports: 
// - wave format
// - a canonical file format (one RIFF chunk, one FMT chunk and one DATA chunk)
//


enum wav_type
{
    WavType_Wave = 0,
    WavType_XWMA,
    WavType_Unknown,
    
    WavType_Count,
};


struct wav_format
{
    u16 Compression;
    u16 NumberOfChannels;
    u32 SampleRate;
    u32 AverageBytesPerSecond;
    u16 BlockAlign;
    u16 SignificantBitsPerSample;
    u16 ExtraFormatBytes;
};


struct wav_data
{
    wav_format Format;
    u8 *Data = nullptr;
    wav_type Type = WavType_Unknown;
    size_t DataSize = 0;
};

b32 ParseWav(u8 *Data, size_t DataSize, wav_data *Output);
void FreeWavData(wav_data *Data);



#endif // #ifndef wav__h