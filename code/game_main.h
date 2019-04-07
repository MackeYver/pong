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

#ifndef game_main__h
#define game_main__h

#include "renderer.h"
#include "audio.h"
#include <vector>
#include <map>



enum shape_type
{
    ShapeType_rectangle,
    ShapeType_circle,
    
    ShapeType_count,
};

struct body;

struct shape
{
    s32 BodyIndex = -1;
    union
    {
        v2 HalfSize = V2(1.0f, 1.0f);
        f32 Radius;
    };
    shape_type Type = ShapeType_rectangle;
};

struct body
{
    shape Shape;
    
    v2 P = v2_zero;
    v2 PrevP = v2_zero;
    
    v2 dP = v2_zero;
    v2 dPMax = V2(f32Max, f32Max);
    v2 dPMask = v2_one;
    
    v2 F = v2_zero;
    f32 Damping = 0.0f;
    f32 InverseMass = 1.0f;
};

struct player
{
    s32 BodyIndex = -1;
    v4 Colour = v4_one;
    u32 Score = 0;
};

struct ball
{
    s32 BodyIndex = -1;
    v4 Colour = v4_one;
};

struct border
{
    s32 BodyIndex = -1;
    v4 Colour = v4_one;
};

enum game_mode
{
    GameMode_Inactive = -1,
    GameMode_Paused   = 0,
    GameMode_Playing,
    GameMode_Scored,
    
    GameMode_Count,
};

struct game_state
{
    renderer *Renderer = nullptr;
    
    audio Audio;
    
    std::map<u8, u8> PressedKeys;
    
    std::vector<body> Bodies;
    player Players[2];
    border Borders[2];
    ball Ball;
    
    v4 BackgroundColour;
    
    u8 PlayerCount;
    game_mode GameMode = GameMode_Inactive;
};


void Init(game_state *State);
void Update(game_state *State, f32 dt);
void Shutdown(game_state *State);


#endif