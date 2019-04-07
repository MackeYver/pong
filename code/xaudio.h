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


#ifndef xaudio__h
#define xaudio__h

#include <xaudio2.h>

#include "win32_file_io.h"
#include "audio.h"
#include "wav.h"



//
// Voice
struct xaudio_voice
{
    wav_data WavData;
    IXAudio2SourceVoice *SourceVoice = nullptr;
    b32 ShouldLoop = false;
};

void Play(xaudio_voice *Voice);
void Stop(xaudio_voice *Voice);



//
// Audio engine
struct xaudio
{
    IXAudio2 *Device = nullptr;
    IXAudio2MasteringVoice *MasteringVoice = nullptr;
    xaudio_voice Voices[Audio_Count];
};

void Init(xaudio *XAudio);
void Shutdown(xaudio *XAudio);

void Play(void *XAudio, audio_sample_id SampleID);
void Stop(void *XAudio, audio_sample_id SampleID);


#endif