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

#include "common.h"


struct vs_input
{
    float2 P : POSITION0;
    float2 T : TEXCOORD0;
};

struct ps_input
{
    float4 P : SV_Position;
    float2 T : TEXCOORD0;
};


Texture2D    gTexture : register(t0);
SamplerState gSampler : register(s0);


//
// Vertex shader
ps_input vMain(vs_input In)
{
    ps_input Result;
    float4x4 ObjectToClip = mul(ObjectToWorld, WorldToClip);
    Result.P = mul(float4(In.P, 0.0f, 1.0f), ObjectToClip);
    Result.T = In.T;
    
    return Result;
}


//
// Pixel shader
float4 pMain(ps_input In) : SV_Target
{
    float4 TextureColour = gTexture.Sample(gSampler, In.T);
    float4 Result = TextureColour * Colour;
    
    return Result;
}