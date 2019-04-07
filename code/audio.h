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


#ifndef audio__h
#define audio__h

enum audio_sample_id
{
    Audio_Theme = 0,
    Audio_PaddleBounce,
    Audio_BorderBounce,
    Audio_Score,
    
    Audio_Count
};

struct audio
{
    void Play(audio_sample_id SampleID)
    {
        _Play(AudioSystem, SampleID);
    }
    
    void Stop(audio_sample_id SampleID)
    {
        _Stop(AudioSystem, SampleID);
    }
    
    void StopAll()
    {
        for (u32 Index = 0; Index < Audio_Count; ++Index)
        {
            Stop((audio_sample_id)Index);
        }
    }
    
    void (*_Play)(void *, audio_sample_id);
    void (*_Stop)(void *, audio_sample_id);
    
    void *AudioSystem;
};

#endif