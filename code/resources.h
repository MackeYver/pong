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

#ifndef resources__h
#define resources__h

#include <vector>

#include "bmp.h"
#include "wav.h"



//
// Textures
// -@debug
//

typedef s32 texture_index;
typedef s32 bmp_data_index;

struct bmp_data
{
    bmp Bmp = {};
    texture_index TextureIndex = -1;
};




//
// Wav
// 

typedef s32 voice_index;
typedef s32 wav_data_index; 

struct wav_data
{
    wav Wav = {};
    voice_index VoiceIndex = -1;
};



//
// Resource management
//

// function pointers to platform specific implementations of render- and audio-systems
struct resources_platform
{
    //
    // Audio
    void *AudioSystem;
    
    voice_index (*_CreateVoice)(void *, wav *);
    voice_index CreateVoice(wav *WAV)
    {
        voice_index Result = _CreateVoice(AudioSystem, WAV);
        return Result;
    }
    
    b32 (*_SetWavDataIndex)(void *, voice_index, wav_data_index);
    b32 SetWavDataIndex(voice_index VoiceIndex, wav_data_index WavDataIndex)
    {
        b32 Result = _SetWavDataIndex(AudioSystem, VoiceIndex, WavDataIndex);
        return Result;
    }
    
    //
    // Rendering
    void *RenderSystem;
    
    texture_index (*_CreateTexture)(void *, bmp *BMP);
    
    texture_index CreateTexture(bmp *BMP)
    {
        texture_index Result = _CreateTexture(RenderSystem, BMP);
        return Result;
    }
};


struct resources
{
    resources_platform Platform;
    
    std::vector<bmp_data> BmpData;
    std::vector<wav_data> WavData;
};


void Init(resources *Resources);
void Shutdown(resources *Resources);

texture_index LoadBMP(resources *Resources, char const *PathAndFileName);

voice_index LoadWAV(resources *Resources, char const *PathAndFileName);
wav_data *GetWavData(resources *Resources, wav_data_index WavDataIndex);


#endif