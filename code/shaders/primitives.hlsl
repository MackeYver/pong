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



//
// Structs
//

cbuffer ConstantBuffer : register(b0)
{
    float4x4 WorldToClip;
    float4x4 ObjectToWorld;
    float4 Colour;
};


struct vs_input
{
    float4 C : Colour0;
    float3 P : Position0;
};


struct vs_output
{
    float4 P : SV_Position;
    float4 C : Colour0;
};




//
// Shaders
//

vs_output vMain(vs_input In)
{
    vs_output Result;
    float4x4 ObjectToClip = mul(ObjectToWorld, WorldToClip);
    Result.P = mul(float4(In.P, 1.0f), ObjectToClip);
    Result.C = In.C;
    
	return Result;
}


float4 pMain(vs_output In) : SV_Target
{
	return In.C;
}