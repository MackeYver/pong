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
#include "resources.h"

#ifdef DEBUG
#include <assert.h>
#include <stdio.h>
#else
#define assert(x) x
#define printf(...)
#endif


//
// Forward declarations
//
//void SubmitSourceBuffer(xaudio_voice *Voice);




//
// Init and shutdown
//

void Init(xaudio *XAudio, resources *Resources)
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
    
    XAudio->Resources = Resources;
}


void Shutdown(xaudio *XAudio)
{
    for (auto& Voice : XAudio->Voices)
    {
        Voice.SourceVoice->Stop();
        Voice.SourceVoice->DestroyVoice();
        Voice.WavResourceIndex = -1;
    }
    XAudio->Voices.clear();
    
    XAudio->MasteringVoice->DestroyVoice();
    XAudio->Device->Release();
}




//
// Resource management
//

voice_index CreateVoice(void *X, wav *WAV)
{
    assert(X);
    
    xaudio *State = reinterpret_cast<xaudio *>(X);
    
    voice_index Result = -1;
    
    if (WAV)
    {
        xaudio_voice NewVoice;
        u32 VoiceIndex = static_cast<u32>(State->Voices.size());
        State->Voices.push_back(NewVoice);
        xaudio_voice *VoicePtr = &State->Voices[VoiceIndex];
        
        tWAVEFORMATEX Format;
        Format.wFormatTag      = WAVE_FORMAT_PCM;
        Format.nChannels       = WAV->Format.NumberOfChannels;
        Format.nSamplesPerSec  = WAV->Format.SampleRate;
        Format.nAvgBytesPerSec = WAV->Format.AverageBytesPerSecond;
        Format.nBlockAlign     = WAV->Format.BlockAlign;
        Format.wBitsPerSample  = WAV->Format.SignificantBitsPerSample;
        Format.cbSize = 0;
        
        Result = State->Device->CreateSourceVoice(&VoicePtr->SourceVoice, (WAVEFORMATEX*)&Format);
        if (FAILED(Result))
        {
            assert(0);
        }
        
        Result = VoiceIndex;
    }
    
    return Result;
}


b32 SetWavResourceIndex(void *X, voice_index VoiceIndex, wav_resource_index WavResourceIndex)
{
    b32 Result = false;
    
    xaudio *State = reinterpret_cast<xaudio *>(X);
    
    if ((VoiceIndex >= 0) && ((u32)VoiceIndex < State->Voices.size()))
    {
        xaudio_voice *Voice = &State->Voices[VoiceIndex];
        Voice->WavResourceIndex = WavResourceIndex;
        Result = true;
    }
    
    return Result;
}




//
// Play and stop functions
// 

void SubmitSourceBuffer(xaudio_voice *Voice, wav *WAV)
{
    //
    // Buffer
    XAUDIO2_BUFFER XAudio2Buffer = {};
    XAudio2Buffer.AudioBytes = WAV->DataSize;
    XAudio2Buffer.pAudioData = WAV->Data;
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
    xaudio *State = (xaudio *)AudioSystem; // Crazy-bananas!
    if ((Index >= 0) && ((u32)Index < State->Voices.size()))
    {
        xaudio_voice *Voice = &State->Voices[Index];
        
        wav_resource *WavResource = GetWavResource(State->Resources, Voice->WavResourceIndex);
        assert(WavResource);
        wav *WAV = &WavResource->Wav;
        
        SubmitSourceBuffer(Voice, WAV); // TODO(Marcus): Find out if we really need to submit audio data evertime we play!
        Voice->SourceVoice->Start();
    }
}


void Stop(void *AudioSystem, voice_index Index)
{
    xaudio *State = (xaudio *)AudioSystem; // BANANAS!
    if ((Index >= 0) && ((u32)Index < State->Voices.size()))
    {
        xaudio_voice *Voice = &State->Voices[Index];
        Voice->SourceVoice->Stop();
        Voice->SourceVoice->FlushSourceBuffers();
    }
}


void StopAll(void *AudioSystem)
{
    xaudio *State = (xaudio *)AudioSystem;
    for (std::vector<xaudio_voice>::size_type Index = 0; 
         Index < State->Voices.size(); 
         ++Index)
    {
        xaudio_voice *Voice = &State->Voices[Index];
        Voice->SourceVoice->Stop();
        Voice->SourceVoice->FlushSourceBuffers();
    }
}
