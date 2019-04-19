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

#ifndef platform__h
#define platform__h

#include "resources.h"
#include "types.h"

struct wav;
struct bmp;
struct mesh;



// function pointers to platform specific implementations of render- and audio-systems
struct platform
{
    b32 (*LoadEntireFile)(char const *PathAndFilename, u8 **OutputData, size_t *OutputDataSize);
    
    
    //
    // Audio
    void *AudioSystem;
    
    s32 (*_CreateVoice)(void *, wav *);
    s32 CreateVoice(wav *WAV)
    {
        s32 Result = _CreateVoice(AudioSystem, WAV);
        return Result;
    }
    
    b32 (*_SetWavResourceIndex)(void *, s32, s32);
    b32 SetWavResourceIndex(s32 VoiceIndex, s32 WavResourceIndex)  
    {
        b32 Result = _SetWavResourceIndex(AudioSystem, VoiceIndex, WavResourceIndex);
        return Result;
    }
    
    
    //
    // Rendering
    void *RenderSystem;
    
    s32 (*_CreateTexture)(void *, bmp *BMP);
    s32 CreateTexture(bmp *BMP)
    {
        s32 Result = _CreateTexture(RenderSystem, BMP);
        return Result;
    }
    
    s32 (*_CreateMesh)(void *, mesh *Mesh);
    s32 CreateMesh(mesh *Mesh)
    {
        s32 Result = _CreateMesh(RenderSystem, Mesh);
        return Result;
    }
};



#endif