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

#include "draw_calls.h"
#include "audio.h"
#include "dynamics.h"
#include "resources.h"

#include <vector>
#include <map>



//
// Entities
//

struct player
{
    body_index BodyIndex = -1;
    v4 Colour = v4_one;
    u32 Score = 0;
};


struct ball
{
    body_index BodyIndex = -1;
    v4 Colour = v4_one;
};


struct border
{
    body_index BodyIndex = -1;
    v4 Colour = v4_one;
};




//
// Game resources
//

// Audio
enum audio_handle // @debug
{
    AudioHandle_Theme = 0,
    AudioHandle_PaddleBounce,
    AudioHandle_BorderBounce,
    AudioHandle_Score,
    
    AudioHandle_Count
};

// Textures
enum texture_handle // @debug
{
    TextureHandle_Ball = 0,
    TextureHandle_Paddle,
    TextureHandle_Brick,
    
    TextureHandle_Count
};

// Meshes
enum mesh_handle // @debug
{
    MeshHandle_Ball = 0,
    MeshHandle_Paddle,
    MeshHandle_Wall,
    
    MeshHandle_Count
};



//
// Game
//

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
    //
    // Resources
    resources Resources;
    
    //
    // Rendering
    draw_calls DrawCalls;
    texture_index TextureHandles[TextureHandle_Count];
    mesh_index MeshHandles[MeshHandle_Count];
    
    //
    // Audio
    audio Audio;
    voice_index AudioHandles[AudioHandle_Count];
    
    //
    // Input
    std::map<u8, u8> PressedKeys; // TODO(Marcus): Implement proper input handling and processing.
    
    //
    // Physics
    dynamics_state Dynamics;
    
    //
    // Entities
    player Players[2];
    border Borders[2];
    ball Ball;
    
    //
    // Game data
    v4 BackgroundColour;
    
    u8 PlayerCount;
    game_mode GameMode = GameMode_Inactive;
};


void Init(game_state *State);
void Update(game_state *State, f32 dt);
void Shutdown(game_state *State);


#endif