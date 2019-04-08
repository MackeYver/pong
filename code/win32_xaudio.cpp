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

#include "win32_xaudio.h"

#ifdef DEBUG
#include <assert.h>
#include <stdio.h>
#else
#define assert(x)
#define printf(...)
#endif


//
// Forward declarations
//
//void SubmitSourceBuffer(xaudio_voice *Voice);




//
// Init and shutdown
//

void Init(xaudio *XAudio)
{
    //
    // XAudio2
    {
        //
        // Device
        HRESULT Result = XAudio2Create(&XAudio->Device, 0, XAUDIO2_DEFAULT_PROCESSOR);
        if (FAILED(Result))
        {
            assert(0);
        }
        
        
        //
        // Master
        Result = XAudio->Device->CreateMasteringVoice(&XAudio->MasteringVoice);
        if (FAILED(Result))
        {
            assert(0);
        }
    }
    
    
#if 0
    char const *FilenameAndPath[] = 
    {
        "data\\theme.wav",
        "data\\paddle_bounce.wav",
        "data\\border_bounce.wav",
        "data\\score.wav",
    };
    
    
    //
    // File read and source voices
    // @debug
    {
        for (u32 Index = 0; Index < Audio_Count; ++Index)
        {
            u8 *Data = nullptr;
            size_t DataSize;
            
            if (!win32_ReadFile(FilenameAndPath[Index], &Data, &DataSize))
            {
                printf("Failed to read file!\n");
                assert(0);
            }
            
            //
            // Parse wav file
            wav_data *WavDataPtr = &XAudio->Voices[Index].WavData;
            b32 Result = ParseWav(Data, DataSize, WavDataPtr);
            assert(Result);
            
            if (Data)
            {
                free(Data);
                Data = nullptr;
                DataSize = 0;
            }
            
            tWAVEFORMATEX Format;
            Format.wFormatTag      = WAVE_FORMAT_PCM;
            Format.nChannels       = WavDataPtr->Format.NumberOfChannels;
            Format.nSamplesPerSec  = WavDataPtr->Format.SampleRate;
            Format.nAvgBytesPerSec = WavDataPtr->Format.AverageBytesPerSecond;
            Format.nBlockAlign     = WavDataPtr->Format.BlockAlign;
            Format.wBitsPerSample  = WavDataPtr->Format.SignificantBitsPerSample;
            Format.cbSize = 0;
            
            xaudio_voice *Voice = &XAudio->Voices[Index];
            Result = XAudio->Device->CreateSourceVoice(&Voice->SourceVoice, (WAVEFORMATEX*)&Format);
            if (FAILED(Result))
            {
                assert(0);
            }
            
            if (Index == (u32)Audio_Theme)
            {
                Voice->ShouldLoop = true;
            }
        }
    }
#endif
}


voice_index Load(void *X, char const *PathAndFilename)
{
    xaudio *XAudio = (xaudio *)X;
    
    voice_index Result = -1;
    
    u8 *Data = nullptr;
    size_t DataSize;
    
    if (win32_ReadFile(PathAndFilename, &Data, &DataSize))
    {
        xaudio_voice NewVoice;
        XAudio->Voices.push_back(NewVoice);
        xaudio_voice *Voice = &XAudio->Voices.back();
        
        wav_data *WavDataPtr = &Voice->WavData;
        b32 ParseResult = ParseWav(Data, DataSize, WavDataPtr);
        assert(ParseResult);
        
        if (Data)
        {
            free(Data);
            Data = nullptr;
            DataSize = 0;
        }
        
        tWAVEFORMATEX Format;
        Format.wFormatTag      = WAVE_FORMAT_PCM;
        Format.nChannels       = WavDataPtr->Format.NumberOfChannels;
        Format.nSamplesPerSec  = WavDataPtr->Format.SampleRate;
        Format.nAvgBytesPerSec = WavDataPtr->Format.AverageBytesPerSecond;
        Format.nBlockAlign     = WavDataPtr->Format.BlockAlign;
        Format.wBitsPerSample  = WavDataPtr->Format.SignificantBitsPerSample;
        Format.cbSize = 0;
        
        Result = XAudio->Device->CreateSourceVoice(&Voice->SourceVoice, (WAVEFORMATEX*)&Format);
        if (FAILED(Result))
        {
            assert(0);
        }
        
#if 0
        if (Index == (u32)Audio_Theme)
        {
            Voice->ShouldLoop = true;
        }
#endif
        
        Result = XAudio->Voices.size() - 1;
    }
    
    return Result;
}


void Shutdown(xaudio *XAudio)
{
#if 0
    for (u32 Index = 0; Index < Audio_Count; ++Index)
    {
        xaudio_voice *Voice = &XAudio->Voices[Index];
        Voice->SourceVoice->Stop();
        Voice->SourceVoice->DestroyVoice();
        FreeWavData(&Voice->WavData);
    }
#endif
    
    for (auto& Voice : XAudio->Voices)
    {
        Voice.SourceVoice->Stop();
        Voice.SourceVoice->DestroyVoice();
        FreeWavData(&Voice.WavData);
    }
    
    XAudio->MasteringVoice->DestroyVoice();
    XAudio->Device->Release();
}



//
// Play and stop functions
// 

void SubmitSourceBuffer(xaudio_voice *Voice)
{
    //
    // Buffer
    XAUDIO2_BUFFER XAudio2Buffer = {};
    XAudio2Buffer.AudioBytes = Voice->WavData.DataSize;
    XAudio2Buffer.pAudioData = Voice->WavData.Data;
    XAudio2Buffer.Flags = XAUDIO2_END_OF_STREAM;
    
    if (Voice->ShouldLoop)
    {
        XAudio2Buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    }
    
    HRESULT Result = Voice->SourceVoice->SubmitSourceBuffer(&XAudio2Buffer);
    if (FAILED(Result))
    {
        assert(0);
    }
}


//
// xaudio
void Play(void *AudioSystem, voice_index Index)
{
    xaudio *XAudio = (xaudio *)AudioSystem; // Crazy-bananas!
    if ((Index >= 0) && ((u32)Index < XAudio->Voices.size()))
    {
        Play(&XAudio->Voices[Index]);
    }
}


void Stop(void *AudioSystem, voice_index Index)
{
    xaudio *XAudio = (xaudio *)AudioSystem; // BANANAS!
    if ((Index >= 0) && ((u32)Index < XAudio->Voices.size()))
    {
        Stop(&XAudio->Voices[Index]);
    }
}


void StopAll(void *AudioSystem)
{
    xaudio *XAudio = (xaudio *)AudioSystem;
    for (std::vector<xaudio_voice>::size_type Index = 0; 
         Index < XAudio->Voices.size(); 
         ++Index)
    {
        Stop(&XAudio->Voices[Index]);
    }
}


//
// xaudio_voice
void Play(xaudio_voice *Voice)
{
    // TODO(Marcus): Find out if we really need to submit the source buffer evertime!
    SubmitSourceBuffer(Voice); // Soooo weird! 
    Voice->SourceVoice->Start();
}


void Stop(xaudio_voice *Voice)
{
    Voice->SourceVoice->Stop();
    Voice->SourceVoice->FlushSourceBuffers();
}