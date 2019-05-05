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

#include "resources.h"



//
// Init and shutdown
//

void Init(resources *Resources)
{
}


void Shutdown(resources *Resources)
{
    //
    // Free bmp
    for (std::vector<bmp_resource>::size_type Index = 0;
         Index < Resources->Bmps.size();
         ++Index)
    {
        Free(&Resources->Bmps[Index].Bmp);
        Resources->Bmps[Index].TextureIndex = -1;
    }
    Resources->Bmps.clear();
    
    
    //
    // Free wav
    for (std::vector<wav_resource>::size_type Index = 0;
         Index < Resources->Wavs.size();
         ++Index)
    {
        Free(&Resources->Wavs[Index].Wav);
        Resources->Wavs[Index].VoiceIndex = -1;
    }
    Resources->Wavs.clear();
    
    
    //
    // Free meshes
    for (std::vector<mesh_resource>::size_type Index = 0;
         Index < Resources->Meshes.size();
         ++Index)
    {
        Free(&Resources->Meshes[Index].Mesh);
        Resources->Meshes[Index].MeshIndex = -1;
    }
    Resources->Meshes.clear();
}




//
// Textures
//

texture_index LoadBMP(resources *Resources, char const *PathAndFileName)
{
    texture_index Result = -1;
    
    u8 *Data = nullptr;
    size_t DataSize = 0;
    b32 Succeeded = Resources->Platform.LoadEntireFile(PathAndFileName, &Data, &DataSize);
    
    bmp_resource BD;
    Succeeded = ParseBMP(Data, DataSize, &BD.Bmp);
    
    if (Data)
    {
        free(Data);
        Data = nullptr;
        DataSize = 0;
    }
    
    if (Succeeded && BD.Bmp.Data, BD.Bmp.DataSize > 0)
    {
        texture_index TextureIndex = Resources->Platform.CreateTexture(&BD.Bmp);
        
        if (TextureIndex >= 0)
        {
            Resources->Bmps.push_back(BD);
            Result = TextureIndex;
        }
        else
        {
            Free(&BD.Bmp);
        }
    }
    
    return Result;
}




//
// Audio
//

voice_index LoadWAV(resources *Resources, char const *PathAndFileName)
{
    voice_index Result = -1;
    
    u8 *Data = nullptr;
    size_t DataSize = 0;
    b32 Succeeded = Resources->Platform.LoadEntireFile(PathAndFileName, &Data, &DataSize);
    
    wav_resource WD;
    Succeeded = ParseWAV(Data, DataSize, &WD.Wav);
    
    if (Data)
    {
        free(Data);
        Data = nullptr;
        DataSize = 0;
    }
    
    if (Succeeded && WD.Wav.Data)
    {
        voice_index VoiceIndex = Resources->Platform.CreateVoice(&WD.Wav);
        
        if (VoiceIndex >= 0)
        {
            Resources->Wavs.push_back(WD);
            Resources->Platform.SetWavResourceIndex(VoiceIndex, Resources->Wavs.size() - 1);
            Result = VoiceIndex;
        }
        else
        {
            Free(&WD.Wav);
        }
    }
    
    return Result;
}


wav_resource *GetWavResource(resources *Resources, wav_resource_index WavResourceIndex)
{
    wav_resource *Result = nullptr;
    
    if ((WavResourceIndex >= 0) && ((u32)WavResourceIndex < Resources->Wavs.size()))
    {
        Result = &Resources->Wavs[WavResourceIndex];
    }
    
    return Result;
}




//
// Mesh
//

mesh_index LoadPLY(resources *Resources, char const *PathAndFilename)
{
    mesh_index Result = -1;
    
    u8 *Data = nullptr;
    size_t DataSize = 0;
    b32 Succeeded = Resources->Platform.LoadEntireFile(PathAndFilename, &Data, &DataSize);
    
    mesh_resource MD;
    Succeeded = ParsePLY(Data, DataSize, &MD.Mesh);
    
    if (Data)
    {
        free(Data);
        Data = nullptr;
        DataSize = 0;
    }
    
    if (Succeeded)
    {
        mesh_index MeshIndex = Resources->Platform.CreateMesh(&MD.Mesh);
        
        if (MeshIndex >= 0)
        {
            Resources->Meshes.push_back(MD);
            Result = MeshIndex;
        }
        else
        {
            Free(&MD.Mesh);
        }
    }
    
    return Result;
}

mesh_index LoadMesh(resources *Resources, v3 *Positions, v2 *UVs, u32 VertexCount)
{
    assert(Positions);
    assert(UVs);
    
    mesh_index Result = -1;
    
    mesh_resource MD;
    
    size_t Size = sizeof(v3) * VertexCount;
    MD.Mesh.Positions = static_cast<v3 *>(malloc(Size));
    assert(MD.Mesh.Positions);
    memcpy(MD.Mesh.Positions, Positions, Size);
    
    Size = sizeof(v2) * VertexCount;
    MD.Mesh.UVs = static_cast<v2 *>(malloc(Size));
    assert(MD.Mesh.UVs);
    memcpy(MD.Mesh.UVs, UVs, Size);
    
    MD.Mesh.VertexCount = VertexCount;
    
    assert(!MD.Mesh.Normals);
    assert(!MD.Mesh.Indices);
    assert(MD.Mesh.IndexCount == 0);
    
    mesh_index MeshIndex = Resources->Platform.CreateMesh(&MD.Mesh);
    if (MeshIndex >= 0)
    {
        Resources->Meshes.push_back(MD);
        Result = MeshIndex;
    }
    else
    {
        Free(&MD.Mesh);
    }
    
    return Result;
}