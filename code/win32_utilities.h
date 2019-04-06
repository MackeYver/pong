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



//
// Window stuff
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
    wchar_t const ClassName[] = L"particles";
    
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