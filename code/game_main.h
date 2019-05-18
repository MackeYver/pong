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

#include "entity_pool.h"
#include "entity.h"
#include "entity_ball.h"
#include "entity_wall.h"
#include "entity_paddle.h"

#include <vector>
#include <map>




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
    mesh_index BackgroundMesh;
    texture_index BackgroundTexture;
    b32 UseRetroMode = false;
    
    
    //
    // Audio
    audio Audio;
    voice_index Audio_Theme;
    voice_index Audio_Score;
    voice_index Audio_PaddleBounce;
    voice_index Audio_WallBounce;
    
    //
    // Input
    std::map<u8, u8> PressedKeys; // TODO(Marcus): Implement proper input handling and processing.
    
    //
    // Physics
    dynamics_state Dynamics;
    
    //
    // Entities
    entity_pool EntityPool;
    entity *Players[2];
    entity *Walls[2];
    entity *Ball;
    u32 Scores[2];
    
    u8 PlayerCount;
    game_mode GameMode = GameMode_Inactive;
};


void Init(game_state *State);
void Update(game_state *State, f32 dt);
void Shutdown(game_state *State);


#endif