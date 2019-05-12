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
#include "mesh.h"
#include "platform.h"
struct resources;



//
// Wav
typedef s32 voice_index;
typedef s32 wav_resource_index; 

struct wav_resource
{
    wav Wav = {};
    voice_index VoiceIndex = -1;
};

voice_index LoadWAV(resources *Resources, char const *PathAndFileName);
wav_resource *GetWavResource(resources *Resources, wav_resource_index WavDataIndex);




//
// Textures
typedef s32 texture_index;
typedef s32 bmp_resource_index;

struct bmp_resource
{
    bmp Bmp = {};
    texture_index TextureIndex = -1;
};

texture_index LoadBMP(resources *Resources, char const *PathAndFileName);




//
// Mesh
typedef s32 mesh_index;
typedef s32 mesh_resource_index;

struct mesh_resource
{
    mesh Mesh = {};
    mesh_index MeshIndex = -1;
};

mesh_index LoadPLY(resources *Resources, char const *PathAndFilename);
mesh_index LoadMesh(resources *Resources, v3 *Positions, v2 *UVs, u32 VertexCount);




//
// Resource management
struct resources
{
    platform Platform;
    
    std::vector<bmp_resource> Bmps;
    std::vector<wav_resource> Wavs;
    std::vector<mesh_resource> Meshes;
};

void Init(resources *Resources);
void Shutdown(resources *Resources);



#endif