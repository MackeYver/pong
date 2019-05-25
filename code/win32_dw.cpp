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

#include "win32_dw.h"
#include "win32_dx.h"

#ifdef DEBUG
#include <assert.h>
#include <stdio.h>
#else
#define assert(x)
#define printf(x)
#endif


b32 Init(dw_state *State, dx_state *DXState)
{
    assert(DXState);
    assert(DXState->Device);
    assert(DXState->SwapChain);
    assert(State);
    
    IDWriteFactory *DWriteFactory;
    HRESULT Result = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                         __uuidof(IDWriteFactory), 
                                         (IUnknown **)&DWriteFactory);
    if (FAILED(Result))
    {
        // TODO(Marcus): Add better error handling
        printf("Failed to create the DirectWrite factory!\n");
        return false;
    }
    
    
    //
    // Direct2D
    D2D1_FACTORY_OPTIONS Options;
    Options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
    ID2D1Factory1 *D2DFactory;
    Result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, // Will only be used in this thread
                               __uuidof(ID2D1Factory),
                               &Options,
                               (void **)&D2DFactory);
    if (FAILED(Result))
    {
        DWriteFactory->Release();
        printf("Failed to create the Direct2D factory!\n");
        return false;
    }
    
    IDXGIDevice *DXGIDevice;
    Result = DXState->Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&DXGIDevice);
    if (FAILED(Result))
    {
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to get the DXGIDevice from the D3D10Device1!\n");
        return false;
    }
    
    Result = D2DFactory->CreateDevice(DXGIDevice, &State->Device);
    if (FAILED(Result))
    {
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DDevice!\n");
        return false;
    }
    
    ID2D1Device *Device = State->Device;
    Result = Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &State->DeviceContext);
    if (FAILED(Result))
    {
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DDeviceContext!\n");
        return false;
    }
    
    IDXGISurface *D2DBuffer;
    //Result = DirectXState->SwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void **)&D2DBuffer);
    Result = DXState->RenderTargetTexture->QueryInterface(&D2DBuffer);
    if (FAILED(Result))
    {
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to get a IDXGISurface from the texture!\n");
        return false;
    }
    
    D2D1_BITMAP_PROPERTIES1 Properties;
    Properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    Properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
    Properties.dpiX = 0;
    Properties.dpiX = 0;
    Properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    Properties.colorContext = nullptr;
    
    ID2D1DeviceContext *DeviceContext = State->DeviceContext;
    Result = DeviceContext->CreateBitmapFromDxgiSurface(D2DBuffer, Properties, &State->RenderTarget);
    if (FAILED(Result))
    {
        D2DBuffer->Release();
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DBitmap!\n");
        return false;
    }
    DeviceContext->SetTarget(State->RenderTarget);
    
    
    
    //
    // Brushes
    Result = DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &State->Brushes[0]);
    if (FAILED(Result))
    {
        State->RenderTarget->Release();
        D2DBuffer->Release();
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DBrush!\n");
        return false;
    }
    
    Result = DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &State->Brushes[1]);
    if (FAILED(Result))
    {
        State->Brushes[0]->Release();
        State->RenderTarget->Release();
        D2DBuffer->Release();
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DBrush!\n");
        return false;
    }
    
    
    
    //
    // Text format
    {
        //
        // Small
        Result = DWriteFactory->CreateTextFormat(L"Bahnschrift", //L"Microsoft New Tai Lue",
                                                 nullptr,
                                                 DWRITE_FONT_WEIGHT_NORMAL,
                                                 DWRITE_FONT_STYLE_NORMAL,
                                                 DWRITE_FONT_STRETCH_NORMAL,
                                                 96.0f / 2.0f,
                                                 L"en-GB",
                                                 &State->TextFormats[0]);
        if (FAILED(Result))
        {
            State->Brushes[0]->Release();
            State->Brushes[1]->Release();
            State->RenderTarget->Release();
            D2DBuffer->Release();
            State->DeviceContext->Release();
            State->Device->Release();
            DXGIDevice->Release();
            D2DFactory->Release();
            DWriteFactory->Release();
            printf("Failed to create the D2DTextFormat!\n");
            return false;
        }
        
        State->TextFormats[0]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        State->TextFormats[0]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        
        //
        // Medium
        Result = DWriteFactory->CreateTextFormat(L"Bahnschrift", //L"Microsoft New Tai Lue",
                                                 nullptr,
                                                 DWRITE_FONT_WEIGHT_NORMAL,
                                                 DWRITE_FONT_STYLE_NORMAL,
                                                 DWRITE_FONT_STRETCH_NORMAL,
                                                 96.0f / 3.0f,
                                                 L"en-GB",
                                                 &State->TextFormats[1]);
        if (FAILED(Result))
        {
            State->Brushes[0]->Release();
            State->Brushes[1]->Release();
            State->TextFormats[0]->Release();
            State->RenderTarget->Release();
            D2DBuffer->Release();
            State->DeviceContext->Release();
            State->Device->Release();
            DXGIDevice->Release();
            D2DFactory->Release();
            DWriteFactory->Release();
            printf("Failed to create the D2DTextFormat!\n");
            return false;
        }
        
        State->TextFormats[1]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        State->TextFormats[1]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        
        //
        // Large
        Result = DWriteFactory->CreateTextFormat(L"Bahnschrift", //L"Microsoft New Tai Lue",
                                                 nullptr,
                                                 DWRITE_FONT_WEIGHT_NORMAL,
                                                 DWRITE_FONT_STYLE_NORMAL,
                                                 DWRITE_FONT_STRETCH_NORMAL,
                                                 96.0f / 4.0f,
                                                 L"en-GB",
                                                 &State->TextFormats[2]);
        if (FAILED(Result))
        {
            State->Brushes[0]->Release();
            State->Brushes[1]->Release();
            State->TextFormats[0]->Release();
            State->TextFormats[1]->Release();
            State->RenderTarget->Release();
            D2DBuffer->Release();
            State->DeviceContext->Release();
            State->Device->Release();
            DXGIDevice->Release();
            D2DFactory->Release();
            DWriteFactory->Release();
            printf("Failed to create the D2DTextFormat!\n");
            return false;
        }
        
        State->TextFormats[2]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        State->TextFormats[2]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
    
    D2DBuffer->Release();
    DXGIDevice->Release();
    D2DFactory->Release();
    DWriteFactory->Release();
    
    return true;
}



void Shutdown(dw_state *State)
{
    assert(State);
    for (int Index = 0; Index < 3; ++Index)
    {
        State->TextFormats[Index] ? State->TextFormats[Index]->Release() : 0;
    }
    
    for (int Index = 0; Index < 2; ++Index)
    {
        State->Brushes[Index] ? State->Brushes[Index]->Release() : 0;
    }
    
    State->RenderTarget ? State->RenderTarget->Release() : 0;
    State->DeviceContext? State->DeviceContext->Release() : 0;
    State->Device ? State->Device->Release() : 0;
}




//
// Rendering
//

void BeginDraw(dw_state *State)
{
    assert(State && State->DeviceContext);
    State->DeviceContext->BeginDraw();
}


HRESULT EndDraw(dw_state *State)
{
    assert(State && State->DeviceContext);
    HRESULT Result = State->DeviceContext->EndDraw();
    
    return Result;
}

void DrawText(dw_state *State, v2 P, wchar_t const *String, size_index Size, colour_index Colour)
{
    assert(State && State->DeviceContext);
    assert(String);
    
    v2 Offset = V2(600.0f, 20.0f);
    D2D1_RECT_F LayoutRect;
    LayoutRect.top    = P.y - Offset.y;
    LayoutRect.left   = P.x - Offset.x;
    LayoutRect.bottom = P.y + Offset.y;
    LayoutRect.right  = P.x + Offset.x;
    
    State->DeviceContext->DrawText(String,
                                   wcslen(String),
                                   State->TextFormats[static_cast<int>(Size)],
                                   &LayoutRect,
                                   State->Brushes[static_cast<int>(Colour)],
                                   D2D1_DRAW_TEXT_OPTIONS_NONE,
                                   DWRITE_MEASURING_MODE_NATURAL);
}