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

#include "win32_dx.h"
#include "win32_dx_texture.h"
#include "bmp.h"


b32 CreateTexture(ID3D11Device *Device, u32 Width, u32 Height, void *Data, u32 RowSize, dx_texture *Output)
{
    //
    // Texture
    D3D11_TEXTURE2D_DESC TexDesc;
    ZeroMemory(&TexDesc, sizeof(D3D11_TEXTURE2D_DESC));
    
    TexDesc.Width = Width;
    TexDesc.Height = Height;
    TexDesc.MipLevels = 1;
    TexDesc.ArraySize = 1;
    TexDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // @debug
    TexDesc.SampleDesc.Count = 1;
    TexDesc.SampleDesc.Quality = 0;
    TexDesc.Usage = D3D11_USAGE_DEFAULT;
    TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    TexDesc.CPUAccessFlags = 0;
    TexDesc.MiscFlags = 0;
    
    HRESULT HResult;
    if (Data && RowSize > 0)
    {
        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
        InitData.pSysMem = Data;
        InitData.SysMemPitch = RowSize;
        InitData.SysMemSlicePitch = 0;
        
        HResult = Device->CreateTexture2D(&TexDesc, &InitData, &Output->Texture);
    }
    else
    {
        HResult = Device->CreateTexture2D(&TexDesc, nullptr, &Output->Texture);
    }
    
    if (FAILED(HResult)) 
    {
        printf("%s: Failed to create the texture.\n", __FILE__);
        return false;
    }
    
    
    //
    // Shader view
    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderViewDesc;
    ShaderViewDesc.Format = TexDesc.Format;
    ShaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ShaderViewDesc.Texture2D.MostDetailedMip = 0;
    ShaderViewDesc.Texture2D.MipLevels = 1;
    
    HResult = Device->CreateShaderResourceView(Output->Texture, &ShaderViewDesc, &Output->ShaderView);
    if (FAILED(HResult)) 
    {
        printf("%s: Failed to create the shader view.\n", __FILE__);
        return false;
    }
    
    return true;
}


b32 CreateTexture(ID3D11Device *Device, u32 Width, u32 Height, dx_texture *Output)
{
    b32 Result = CreateTexture(Device, Width, Height, nullptr, 0, Output);
    
    return Result;
}


s32 CreateTexture(void *DXState, bmp *BMP)
{
    dx_state *State = reinterpret_cast<dx_state *>(DXState); // CRAZY!
    s32 Result = -1;
    
    if (BMP)
    {
        dx_texture T;
        b32 CreateResult = CreateTexture(State->Device, BMP->Header.Width, BMP->Header.Height, BMP->Data, BMP->RowSize, &T);
        
        if (!CreateResult) 
        {
            printf("%s: Failed to create the texture.\n", __FILE__);
        }
        else
        {
            State->Textures.push_back(T);
            Result = static_cast<s32>(State->Textures.size()) - 1;
        }
    }
    
    return Result;
}



void Free(dx_texture *Texture)
{
    if (Texture)
    {
        Texture->ShaderView ? Texture->ShaderView->Release() : 0;
        Texture->Texture ? Texture->Texture->Release() : 0;
    }
}
