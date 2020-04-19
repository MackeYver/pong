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
#include "resources.h"

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
    dx_state DirectX;
    
    game_state GameState;
};




//
// Window utilities
//

// Updates the MouseP in the app_state to the current position of the mouse in screen space,
// with the origin in the bottom left of the screen.
static void UpdateMouseState(mouse_state *State)
{
    POINT Point;
    GetCursorPos(&Point);
    State->PrevP = State->P;
    State->P.x = (f32)Point.x;
    State->P.y = (f32)(State->DisplayMetrics->ScreenHeight - Point.y - 1); // -1 due to being zero based
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (uMsg) {
        case WM_CREATE: 
        {
            // Get a pointer to the app_state struct and store it in the instance data of the window
            CREATESTRUCT *Create = reinterpret_cast<CREATESTRUCT *>(lParam);
            app_state *State = reinterpret_cast<app_state *>(Create->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)State);
        } break;
        
        case WM_KEYDOWN: 
        {
            LONG_PTR Ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
            app_state *State = reinterpret_cast<app_state *>(Ptr);
            
            ++State->GameState.PressedKeys[(u8)wParam];
        } break;
        
        case WM_KEYUP: 
        {
            LONG_PTR Ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
            app_state *State = reinterpret_cast<app_state *>(Ptr);
            
            if (State->GameState.PressedKeys.count((u8)wParam) > 0)
            {
                State->GameState.PressedKeys.erase((u8)wParam);
            }
        } break;
        
        case WM_RBUTTONDOWN: 
        {
            LONG_PTR Ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
            app_state *AppState = reinterpret_cast<app_state *>(Ptr);
            
            mouse_state *MouseState = &AppState->MouseState;
            MouseState->RBDown = true;
            UpdateMouseState(MouseState);
            
            SetCapture(hWnd);
        } break;
        
        case WM_RBUTTONUP: 
        {
            LONG_PTR Ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
            app_state *AppState = reinterpret_cast<app_state *>(Ptr);
            
            mouse_state *MouseState = &AppState->MouseState;
            MouseState->RBDown = false;
            
            ReleaseCapture();
        } break;
        
        
        case WM_DESTROY: 
        {
            PostQuitMessage(0);
            return 0;
        } break;
        
        default: {
            //DebugPrint(L"Message = %d\n", uMsg);
        } break;
    }
    
    
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static u32 CreateAppWindow(app_state *State, HINSTANCE hInstance, WNDPROC WindowProc)
{
    wchar_t const ClassName[] = L"pong";
    
    WNDCLASS WC;
    
    WC.style = CS_VREDRAW | CS_HREDRAW;
    WC.lpfnWndProc = WindowProc;
    WC.cbClsExtra = 0;
    WC.cbWndExtra = 0;
    WC.hInstance = hInstance;
    WC.hIcon = nullptr;
    WC.hCursor = LoadCursor(nullptr, IDC_ARROW);
    WC.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WC.lpszMenuName = nullptr;
    WC.lpszClassName = ClassName;
    
    DWORD Style =  WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    
    RECT Rect;
    Rect.left = 0;
    Rect.top = 0;
    Rect.right = State->DisplayMetrics.WindowWidth;
    Rect.bottom = State->DisplayMetrics.WindowHeight;
    AdjustWindowRect(&Rect, Style, false);
    
    if (!RegisterClass(&WC)) 
    {
        return 1;
    }
    
    State->hWnd = CreateWindow(ClassName,                      /* Class Name */
                               ClassName,                      /* Window name */
                               Style,                          /* Style */
                               CW_USEDEFAULT, CW_USEDEFAULT,   /* Position */
                               Rect.right - Rect.left, 
                               Rect.bottom - Rect.top ,        /* Size */
                               nullptr,                        /* Parent */
                               nullptr,                        /* No menu */
                               hInstance,                      /* Instance */
                               State);                         /* This pointer is passed via the WM_CREATE message */
    
    if (!State->hWnd)
    {
        DWORD Error = GetLastError();
        return Error;
    }
    
    return 0;
}




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
    // Set up function pointers
    //
    
    {
        AppState.GameState.Resources.Platform.LoadEntireFile = &win32_ReadFile;
        
        AppState.GameState.Audio.AudioSystem = (void *)&AppState.XAudio;
        AppState.GameState.Resources.Platform.AudioSystem = (void *)&AppState.XAudio;
        AppState.GameState.Resources.Platform._CreateVoice = &CreateVoice;
        AppState.GameState.Resources.Platform._SetWavResourceIndex = &SetWavResourceIndex;
        
        AppState.GameState.Resources.Platform.RenderSystem = (void *)&AppState.DirectX;
        AppState.GameState.Resources.Platform._CreateTexture = &CreateTexture;
        AppState.GameState.Resources.Platform._CreateMesh = &CreateMesh;
        
        AppState.GameState.Audio._Play = &Play;
        AppState.GameState.Audio._Stop = &Stop;
        AppState.GameState.Audio._StopAll = &StopAll;
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
    // XAudio
    //
    
    {
        HRESULT Result = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(Result))
        {
            assert(0);
        }
        
        Init(&AppState.XAudio, &AppState.GameState.Resources);
    }
    
    
    
    //
    // DirectX 10
    //
    {
        b32 BoolResult = Init(&AppState.DirectX, AppState.hWnd);
        assert(BoolResult);
    }
    
    
    
    //
    // Init game
    //
    {
        //
        // Init subsystems
        Init(&AppState.GameState.Resources);
        Init(&AppState.GameState.DrawCalls, 1 << 20, AppState.DisplayMetrics);
        Init(&AppState.GameState.Audio);
        Init(&AppState.GameState.Dynamics);
        
        //
        // Init game
        Init(&AppState.GameState);
    }
    
    
    
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
        ProcessDrawCalls(&AppState.DirectX, &AppState.GameState.DrawCalls);
        
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
    Shutdown(&AppState.GameState.Dynamics);
    Shutdown(&AppState.GameState.Audio);
    Shutdown(&AppState.GameState.DrawCalls);
    Shutdown(&AppState.GameState.Resources);
    
    CoUninitialize();
    Shutdown(&AppState.XAudio);
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
    
    
    return 0;
}
