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


//
// NOTE(Marcus): First steps towards a platform agnostic audio layer
//


typedef s32 voice_index;


struct audio
{
    void Init()
    {
    }
    
    void Shutdown()
    {
        StopAll();
    }
    
    void Play(voice_index Index)
    {
        _Play(AudioSystem, Index);
    }
    
    void Stop(voice_index Index)
    {
        _Stop(AudioSystem, Index);
    }
    
    void StopAll()
    {
        _StopAll(AudioSystem);
    }
    
    voice_index Load(char const *PathAndFilename)
    {
        voice_index Result = _Load(AudioSystem, PathAndFilename);
        return Result;
    }
    
    void (*_Play)(void *, voice_index);
    void (*_Stop)(void *, voice_index);
    void (*_StopAll)(void *);
    voice_index (*_Load)(void *, char const *PathAndFilename);
    
    void *AudioSystem;
};



#endif