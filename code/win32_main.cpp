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

#define UNICODE
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <assert.h>
#include <stdio.h>

#include "types.h"
#include "mathematics.h"

#include "win32_xaudio.h"
#include "win32_dx.h"
#include "draw_calls.h"

#include "game_main.h"


f32 constexpr kFrameTime = 1.0f / 60.0f;
f32 constexpr kFrameTimeMicroSeconds = 1000000.0f * kFrameTime;



//
// Structs
//

struct mouse_state
{
    display_metrics *DisplayMetrics;
    
    v2  P = v2_zero;
    v2  PrevP = v2_zero;
    
    b32 RBDown;
};


struct app_state
{
    mouse_state MouseState;
    display_metrics DisplayMetrics;
    HWND hWnd;
    
    xaudio XAudio;
    draw_calls DrawCalls;
    dx_state DirectX;
    
    game_state GameState;
};

#include "win32_utilities.h"




//
// Main
//

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hInstancePrev, LPSTR lpCmdLine, int nShowCmd) 
{
    //
    // Create and attach console
#ifdef DEBUG
    FILE *FileStdOut;
    assert(AllocConsole());
    freopen_s(&FileStdOut, "CONOUT$", "w", stdout);
#endif
    
    
    
    //
    // Display metrics
    //
    
    app_state AppState;
    AppState.DisplayMetrics.WindowWidth  = 1920;
    AppState.DisplayMetrics.WindowHeight = 1080;
    AppState.DisplayMetrics.ScreenWidth  = GetSystemMetrics(SM_CXSCREEN);
    AppState.DisplayMetrics.ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    AppState.MouseState.DisplayMetrics = &AppState.DisplayMetrics;
    
    
    
    //
    // XAudio
    //
    
    {
        HRESULT Result = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(Result))
        {
            assert(0);
        }
        
        Init(&AppState.XAudio);
        AppState.GameState.Audio.AudioSystem = (void *)&AppState.XAudio;
        AppState.GameState.Audio._Play = &Play;
        AppState.GameState.Audio._Stop = &Stop;
        AppState.GameState.Audio._StopAll = &StopAll;
        AppState.GameState.Audio._Load = &Load;
    }
    
    
    
    //
    // Create window
    //
    
    {
        u32 Error = CreateAppWindow(&AppState, hInstance, WindowProc);
        if (Error)
        {
            OutputDebugString(L"Failed to create window!\n");
            return Error;
        }
    }
    
    
    
    //
    // Init DirectX 10
    //
    {
        b32 BoolResult = Init(&AppState.DirectX, AppState.hWnd);
        assert(BoolResult);
    }
    
    
    
    //
    // DirectWrite
    //
    
#if 0
    directwrite_state DirectWriteState;
    InitDirectWrite(&DirectXState, &DirectWriteState);
#endif
    
    
    
    //
    // Init game
    //
    
    Init(&AppState.DrawCalls, 1 << 20, AppState.DisplayMetrics);
    AppState.GameState.DrawCalls = &AppState.DrawCalls;
    Init(&AppState.GameState);
    
    
    
    //
    // Show and update window
    //
    
    ShowWindow(AppState.hWnd, nShowCmd);
    UpdateWindow(AppState.hWnd);
    
    
    
    //
    // Timing
    //
    
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency); 
    
    LARGE_INTEGER RunTime;
    RunTime.QuadPart = 0;
    u32 FrameCount = 0;
    
    
    
    //
    // The main loop
    //
    
    b32 ShouldRun = true;
    MSG msg;
    while (ShouldRun) 
    {
        LARGE_INTEGER FrameStartTime;
        QueryPerformanceCounter(&FrameStartTime);
        
        //
        // Handle the messages from the OS
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            if (msg.message == WM_QUIT) 
            {
                ShouldRun = false;
            }
        }
        
        //
        // Input
        UpdateMouseState(&AppState.MouseState);
        
        //
        // Update
        Update(&AppState.GameState, kFrameTime);
        
        //
        // Process draw calls
        ProcessDrawCalls(&AppState.DirectX, &AppState.DrawCalls);
        
        //
        // Timing
        LARGE_INTEGER FrameEndTime;
        QueryPerformanceCounter(&FrameEndTime);
        
        LARGE_INTEGER ElapsedMicroseconds;
        ElapsedMicroseconds.QuadPart = FrameEndTime.QuadPart - FrameStartTime.QuadPart;
        ElapsedMicroseconds.QuadPart *= 1000000;
        ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
        
        while (ElapsedMicroseconds.QuadPart < kFrameTimeMicroSeconds)
        {
            QueryPerformanceCounter(&FrameEndTime);
            
            ElapsedMicroseconds.QuadPart = FrameEndTime.QuadPart - FrameStartTime.QuadPart;
            ElapsedMicroseconds.QuadPart *= 1000000;
            ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
        }
        
        ++FrameCount;
        
        RunTime.QuadPart += ElapsedMicroseconds.QuadPart;
        if (RunTime.QuadPart > 1000000)
        {
            RunTime.QuadPart = 0;
            FrameCount = 0;
        }
    }
    
    
    
    //
    // @debug
    //
    
#ifdef DEBUG
    IDXGIDebug *DebugInterface;
    {
        HMODULE hModule = GetModuleHandleA("Dxgidebug.dll");
        if (!hModule) {
            OutputDebugString(L"Failed to get module handle to Dxgidebug.dll\n");
            return 1;
        }
        
        typedef HRESULT (*GETDEBUGINTERFACE)(REFIID, void **);
        GETDEBUGINTERFACE GetDebugInterface;
        GetDebugInterface = (GETDEBUGINTERFACE)GetProcAddress(hModule, "DXGIGetDebugInterface");
        
        HRESULT Result = GetDebugInterface(__uuidof(IDXGIDebug), (void **)&DebugInterface);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to get the debug interface!\n");
            return 1;
        }
    }
#endif
    
    
    
    //
    // Clean up
    //
    
    Shutdown(&AppState.GameState);
    Shutdown(&AppState.DrawCalls);
    
    CoUninitialize();
    Shutdown(&AppState.XAudio);
    
    //ReleaseDirectWrite(&DirectWriteState);
    //ReleaseDirectXState(&DirectXState);
    Shutdown(&AppState.DirectX);
    
    
    
#ifdef DEBUG
    OutputDebugString(L"***** Begin ReportLiveObjects call *****\n");
    DebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
    OutputDebugString(L"***** End   ReportLiveObjects call *****\n\n");
    
    if (DebugInterface) {
        DebugInterface->Release();
    }
    
    FreeConsole();
#endif
    
    
    return msg.wParam;
}