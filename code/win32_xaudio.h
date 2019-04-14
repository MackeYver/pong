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


#ifndef win32_xaudio__h
#define win32_xaudio__h

#include <xaudio2.h>
#include <vector> //@debug

#include "win32_file_io.h"
#include "audio.h"
#include "wav.h"
#include "resources.h"



//
// Voice
//

struct xaudio_voice
{
    wav_data_index WavDataIndex = -1;
    IXAudio2SourceVoice *SourceVoice = nullptr;
    b32 ShouldLoop = false;
};



//
// Audio engine
//

struct xaudio
{
    resources *Resources;
    IXAudio2 *Device = nullptr;
    IXAudio2MasteringVoice *MasteringVoice = nullptr;
    std::vector<xaudio_voice> Voices;
};

void Init(xaudio *XAudio, resources *Resources);
void Shutdown(xaudio *XAudio);

voice_index CreateVoice(void *XAudio, wav *Wav);
b32 SetWavDataIndex(void *XAudio, voice_index VoiceIndex, wav_data_index WavDataIndex);

void Play(void *XAudio, voice_index Index);
void Stop(void *XAudio, voice_index Index);
void StopAll(void *XAudio);


#endif
