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

#include "wav.h"
#include "win32_file_io.h"

#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif

#define UNUSED_VAR(x) x



//
// Chunks fourcc
//

u32 constexpr kWavFourCC_RIFF = 'FFIR';
u32 constexpr kWavFourCC_DATA = 'atad';
u32 constexpr kWavFourCC_FMT  = ' tmf';
u32 constexpr kWavFourCC_WAVE = 'EVAW';
u32 constexpr kWavFourCC_XWMA = 'AMWX';




//
// Structs
//

struct wav_data_info
{
    u8 *StartOfBuffer = nullptr;
    size_t CurrentIndex = 0;
    size_t SizeOfBuffer = 0;
    b32 Error = false;
};




//
// Helper funtions
//

b32 IsEOF(wav_data_info *Data, u32 BytesToAdvance = 0)
{
    b32 Result = false;
    
    if (!Data->StartOfBuffer || ((Data->CurrentIndex + BytesToAdvance) > Data->SizeOfBuffer))
    {
        Data->Error = true;
        Result = true;
    }
    
    return Result;
}


b32 IsValid(wav_data_info *Data)
{
    b32 Result = false;
    
    if (!Data->Error || !IsEOF(Data))
    {
        Result = true;
    }
    
    return Result;
}


void Advance(wav_data_info *Data, u32 BytesToAdvance)
{
    if (!IsEOF(Data, BytesToAdvance))
    {
        Data->CurrentIndex += BytesToAdvance;
    }
}


u8 *GetCurrentAddress(wav_data_info *Data)
{
    u8 *Result = nullptr;
    
    if (IsValid(Data))
    {
        Result = Data->StartOfBuffer + Data->CurrentIndex;
    }
    
    return Result;
}


u32 Peek_u32(wav_data_info *Data)
{
    u32 Result = 0;
    
    if (IsValid(Data))
    {
        Result = *((u32 *)GetCurrentAddress(Data));
    }
    
    return Result;
}


u32 Get_u32(wav_data_info *Data)
{
    u32 Result = 0;
    
    if (IsValid(Data))
    {
        Result = *((u32 *)GetCurrentAddress(Data));
        Advance(Data, 4);
    }
    
    return Result;
}


u16 Get_u16(wav_data_info *Data)
{
    u16 Result = 0;
    
    if (IsValid(Data))
    {
        Result = *((u16 *)GetCurrentAddress(Data));
        Advance(Data, 2);
    }
    
    return Result;
}


b32 ParseRIFF(wav_data_info *Data, wav *Output)
{
    b32 Result = false;
    
    if (IsValid(Data))
    {
        u32 FourCC = Peek_u32(Data);
        if (FourCC == kWavFourCC_RIFF)
        {
            Advance(Data, 4);
            
            u32 FileSize = Get_u32(Data);
            UNUSED_VAR(FileSize);
            // From MSDN: "The value of fileSize includes the size of fileType FOURCC plus the size of the data that follows, but
            // does not include the size of the "RIFF" FOURCC or the size of fileSize. The data consists of chunks in any order".
            //Output->FileSize = FileSize - ? or +, we probably don't need to store this
            
            u32 FileType = Get_u32(Data);
            if (FileType == kWavFourCC_WAVE) // We only support wav for now
            {
                Output->Type = WavType_Wave;
                Result = true;
            }
#if 0
            else if (FileType == kWavFourCC_XWMA) // We only support wav for now
            {
                Output->Type = WavType_XWMA;
                Result = true;
            }
#endif
        }
    }
    
    Data->Error = Result;
    return Result;
}


b32 ParseFMT_(wav_data_info *Data, wav *Output)
{
    b32 Result = false;
    
    if (IsValid(Data))
    {
        u32 FourCC = Peek_u32(Data);
        if (FourCC == kWavFourCC_FMT)
        {
            Advance(Data, 4);
            
            u32 ChunkSize = Get_u32(Data);
            UNUSED_VAR(ChunkSize);
            
            wav_format *Fmt = &Output->Format;
            Fmt->Compression = Get_u16(Data);
            Fmt->NumberOfChannels = Get_u16(Data);
            Fmt->SampleRate = Get_u32(Data);
            Fmt->AverageBytesPerSecond = Get_u32(Data);
            Fmt->BlockAlign = Get_u16(Data);
            Fmt->SignificantBitsPerSample = Get_u16(Data);
            
            if (ChunkSize > 16) // Extra bytes maybe, should only be in the case of compressed data
            {
                Fmt->ExtraFormatBytes = Get_u16(Data);
                Advance(Data, Fmt->ExtraFormatBytes); // We don't care about the extra bytes for now...
            }
            
            if (Fmt->Compression != 1) // We only support uncompressed data for the moment.
            {
                assert(0);
            }
            
            if (Fmt->SignificantBitsPerSample % 8 != 0) // Byte aligned?
            {
                assert(0);
            }
            
            Result = true;
        }
    }
    
    Data->Error = Result;
    return Result;
}


b32 ParseDATA(wav_data_info *Data, wav *Output)
{
    b32 Result = false;
    
    if (IsValid(Data))
    {
        u32 FourCC = Peek_u32(Data);
        if (FourCC == kWavFourCC_DATA)
        {
            Advance(Data, 4);
            
            Output->DataSize = Get_u32(Data);
            Output->Data = static_cast<u8 *>(malloc(Output->DataSize));
            assert(Output->Data);
            
            memcpy(Output->Data, GetCurrentAddress(Data), Output->DataSize);
            
            Result = true;
        }
    }
    
    return Result;
}




//
// "Public" functions
//

b32 Load(char const *PathAndFilename, wav *Output)
{
    b32 Result = false;
    
    if (Output)
    {
        u8 *Data = nullptr;
        size_t DataSize;
        
        Result = win32_ReadFile(PathAndFilename, &Data, &DataSize);
        
        if (Result && Data)
        {
            wav_data_info DataInfo;
            DataInfo.StartOfBuffer = Data;
            DataInfo.CurrentIndex = 0;
            DataInfo.SizeOfBuffer = DataSize;
            
            Result = ParseRIFF(&DataInfo, Output);
            assert(Result);
            
            Result = ParseFMT_(&DataInfo, Output);
            assert(Result);
            
            Result = ParseDATA(&DataInfo, Output);
            assert(Result);
            
            if (Data)
            {
                free(Data);
                Data = nullptr;
                DataSize = 0;
            }
        }
    }
    
    return Result;
}


#if 0
b32 ParseWav(u8 *InputData, size_t InputDataSize, wav *Output)
{
    assert(InputData);
    assert(InputDataSize > 0);
    assert(Output);
    
    wav_data_info Data;
    Data.StartOfBuffer = InputData;
    Data.CurrentIndex = 0;
    Data.SizeOfBuffer = InputDataSize;
    
    b32 Result = false;
    Result = ParseRIFF(&Data, Output);
    assert(Result);
    
    Result = ParseFMT_(&Data, Output);
    assert(Result);
    
    Result = ParseDATA(&Data, Output);
    assert(Result);
    
    return Result;
}
#endif


void Free(wav *Data)
{
    if (Data)
    {
        if (Data->Data)
        {
            free(Data->Data);
            memset(Data, 0, sizeof(wav));
        }
    }
}