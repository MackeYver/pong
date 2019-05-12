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

#ifndef shader_final__h
#define shader_final__h

#include "mathematics.h"
#include "win32_dx_buffer.h"

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct dx_texture;


struct shader_final
{
    ID3D11VertexShader *VertexProgram = nullptr;
    ID3D11PixelShader *PixelProgram = nullptr;
    ID3D11InputLayout *InputLayout = nullptr;
    ID3D11SamplerState *Sampler = nullptr;
    dx_buffer VertexBuffer;
};


b32 Init(ID3D11Device *Device, shader_final *Shader);
void Shutdown(shader_final *Shader);

void Use(ID3D11DeviceContext *DC, shader_final *Shader);
void DrawTextureToScreen(ID3D11DeviceContext *DC, shader_final *Shader, dx_texture *Texture);

#endif