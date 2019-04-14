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
    for (std::vector<bmp_data>::size_type Index = 0;
         Index < Resources->BmpData.size();
         ++Index)
    {
        Free(&Resources->BmpData[Index].Bmp);
        Resources->BmpData[Index].TextureIndex = -1;
    }
    Resources->BmpData.clear();
    
    for (std::vector<wav_data>::size_type Index = 0;
         Index < Resources->WavData.size();
         ++Index)
    {
        Free(&Resources->WavData[Index].Wav);
        Resources->WavData[Index].VoiceIndex = -1;
    }
    Resources->WavData.clear();
}




//
// Textures
//

texture_index LoadBMP(resources *Resources, char const *PathAndFileName)
{
    texture_index Result = -1;
    
    bmp_data BD;
    b32 Succeeded = Load(PathAndFileName, &BD.Bmp);
    
    if (Succeeded && BD.Bmp.Data)
    {
        texture_index TextureIndex = Resources->Platform.CreateTexture(&BD.Bmp);
        
        if (TextureIndex >= 0)
        {
            Resources->BmpData.push_back(BD);
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
    
    wav_data WD;
    b32 Succeeded = Load(PathAndFileName, &WD.Wav);
    
    if (Succeeded && WD.Wav.Data)
    {
        voice_index VoiceIndex = Resources->Platform.CreateVoice(&WD.Wav);
        
        if (VoiceIndex >= 0)
        {
            Resources->WavData.push_back(WD);
            Resources->Platform.SetWavDataIndex(VoiceIndex, Resources->WavData.size() - 1);
            Result = VoiceIndex;
        }
        else
        {
            Free(&WD.Wav);
        }
    }
    
    return Result;
}


wav_data *GetWavData(resources *Resources, wav_data_index WavDataIndex)
{
    wav_data *Result = nullptr;
    
    if ((WavDataIndex >= 0) && ((u32)WavDataIndex < Resources->WavData.size()))
    {
        Result = &Resources->WavData[WavDataIndex];
    }
    
    return Result;
}