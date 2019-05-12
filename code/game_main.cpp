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

#include "game_main.h"
#include <stdio.h>
#include <time.h>

#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif




//
// Forward declarations
//

void Render(game_state *State);
void ProcessInput(game_state *State);

void ResetPositions(game_state *State);
void NewGame(game_state *State);
void Start(game_state *State);
void Pause(game_state *State);
void Score(game_state *State, u32 ScoringPlayerIndex);
void End(game_state *State);




//
// "Public" functions
//

void Init(game_state *State)
{
    assert(State);
    
    State->GameMode = GameMode_Inactive;
    State->Scores[0] = 0;
    State->Scores[1] = 0;
    
    
    //
    // Setup entities
    InitEntities(State);
    ResetPositions(State);
    
    
    //
    // Game audio
    {
        voice_index VIndex = LoadWAV(&State->Resources, "data\\theme.wav");
        assert(VIndex >= 0);
        State->Audio_Theme = VIndex;
        
        VIndex = LoadWAV(&State->Resources, "data\\score.wav");
        assert(VIndex >= 0);
        State->Audio_Score = VIndex;
    }
    
    //
    // Background texture and mesh
    {
        // Texture
        texture_index TIndex = LoadBMP(&State->Resources, "data\\background.bmp");
        assert(TIndex >= 0);
        State->BackgroundTexture = TIndex;
        
        // Mesh
        f32 const w = (f32)State->DrawCalls.DisplayMetrics.WindowWidth;
        f32 const h = (f32)State->DrawCalls.DisplayMetrics.WindowHeight;
        
        v3 P[] =
        {
            {0.0f,    h, 0.0f},
            {0.0f, 0.0f, 0.0f},
            {+w  , 0.0f, 0.0f},
            
            {0.0f,    h, 0.0f},
            {   w, 0.0f, 0.0f},
            {   w,    h, 0.0f},
        };
        
        f32 s = w / 128.0f;
        f32 t = h / 128.0f;
        
        v2 UV[] =
        {
            {0.0f, 0.0f},
            {0.0f,    t},
            {s   ,    t},
            
            {0.0f, 0.0f},
            {   s,    t},
            {   s, 0.0f},
        };
        
        mesh_index MIndex = LoadMesh(&State->Resources, P, UV, 6);
        assert(MIndex >= 0);
        State->BackgroundMesh = MIndex;
    }
    
    
    //
    // Game state
    State->PlayerCount = 0;
    
    
    //
    // Seed the RNG
    {
        time_t Time = time(nullptr) * time(nullptr);
        srand(static_cast<unsigned int>(Time));
    }
    
    State->Audio.Play(State->Audio_Theme);
}


void UpdateAI(game_state *State, f32 dt)
{
    if (State->PlayerCount == 2)
    {
        return;
    }
    
    body *BallBody = GetBody(&State->Dynamics, State->Ball->BodyIndex);
    f32 LengthFactor = 0.8f;
    
    //
    // Left paddle, only if we're 0 players
    if (State->PlayerCount == 0)
    {
        body *PaddleBody = GetBody(&State->Dynamics, State->Players[0]->BodyIndex);
        
        State->PressedKeys.erase(0x57);
        State->PressedKeys.erase(0x53);
        
        f32 Length = LengthFactor * PaddleBody->Shape.HalfSize.y;
        
        if (BallBody->P.y > (PaddleBody->P.y + Length))
        {
            State->PressedKeys[0x57] = 1;
        }
        else if (BallBody->P.y < (PaddleBody->P.y - Length))
        {
            State->PressedKeys[0x53] = 1;
        }
    }
    
    
    //
    // Right paddle, if < 2 players
    {
        body *PaddleBody = GetBody(&State->Dynamics, State->Players[1]->BodyIndex);
        
        State->PressedKeys.erase(0x28);
        State->PressedKeys.erase(0x26);
        
        f32 Length = LengthFactor * PaddleBody->Shape.HalfSize.y;
        
        if (BallBody->P.y > (PaddleBody->P.y + Length))
        {
            State->PressedKeys[0x26] = 1;
        }
        else if (BallBody->P.y < (PaddleBody->P.y - Length))
        {
            State->PressedKeys[0x28] = 1;
        }
    }
}


void Update(game_state *State, f32 dt)
{
    assert(State);
    assert(dt > 0.0f);
    
    
    //
    // Process inputs
    ProcessInput(State);
    
    
    if (State->GameMode == GameMode_Playing)
    {
        //
        // "AI"
        UpdateAI(State, dt);
        
        
        //
        // "Physics"
        Update(&State->Dynamics, dt);
        
        std::vector<collision_info> static Collisions;
        Collisions.clear();
        
        DetectCollisions(&State->Dynamics, Collisions);
        
        body *BallBody = GetBody(&State->Dynamics, State->Ball->BodyIndex);
        
        {
            f32 z = 0.8f;
            v3 P0 = V3(0.0f, 0.0f, z);
            v3 P1 = V3(BallBody->P, z);
            PushPrimitiveLine(&State->DrawCalls, P0, P1, V4(1.0f, 1.0f, 0.0f, 1.0f));
        }
        
        for (auto& Collision : Collisions)
        {
            //
            // Will be used in order to determine if we shall play any sounds
            b32 TheBallIsInvolved = false;
            b32 ABorderIsInvolved = false;
            b32 APlayerIsInvolved = false;
            
            
            //
            // Check what entities were involved in the collision
            if ((Collision.BodiesInvolved[0] == State->Ball->BodyIndex) || (Collision.BodiesInvolved[1] == State->Ball->BodyIndex))
            {
                TheBallIsInvolved = true;
            }
            
            for (u32 Index = 0; Index < 2; ++Index)
            {
                if ((Collision.BodiesInvolved[0] == State->Players[Index]->BodyIndex) ||
                    (Collision.BodiesInvolved[1] == State->Players[Index]->BodyIndex))
                {
                    APlayerIsInvolved = true;
                }
                
                if ((Collision.BodiesInvolved[0] == State->Walls[Index]->BodyIndex) ||
                    (Collision.BodiesInvolved[1] == State->Walls[Index]->BodyIndex))
                {
                    ABorderIsInvolved = true;
                }
            }
            
            
            //
            // Play sounds
            if (TheBallIsInvolved && APlayerIsInvolved)
            {
                State->Audio.Play(State->Players[0]->Audio_BallBounce);
                Collision.ForceModifier = 0.15f;
            }
            else if (TheBallIsInvolved && ABorderIsInvolved)
            {
                State->Audio.Play(State->Walls[0]->Audio_BallBounce);
                
                f32 L = Length(BallBody->dP);
                if (L > 900.0f)
                {
                    Collision.ForceModifier = -0.15f;
                }
            }
            else if (APlayerIsInvolved && ABorderIsInvolved)
            {
                Collision.SkipForceApplication = true;
            }
        }
        
        ResolveCollisions(&State->Dynamics, Collisions, dt);
        
        //
        // Check if any of the players scored.
        {
            if (BallBody->P.x < 0.0f)
            {
                Score(State, 1);
            }
            else if (BallBody->P.x > State->DrawCalls.DisplayMetrics.WindowWidth)
            {
                Score(State, 0);
            }
        }
    }
    
    
    //
    // Update entities
    {
        for (u32 Index = 0; Index < 5; ++Index)
        {
            entity *Entity = &State->Entities[Index];
            Update(&State->Dynamics, Entity);
        }
    }
    
    
    //
    // Rendering
    Render(State);
}


void Shutdown(game_state *State)
{
    assert(State);
}




//
// Change game mode
//

void ServeBoll(game_state *State)
{
    //
    // Random starting angle
    u32 dT = 40;
    f32 Theta = Pi32 * ((f32)dT / 180.0f);
    
    u32 RandInt = rand() % dT;
    f32 Angle = Pi32 * ((f32)RandInt / 180.0f);
    
    u32 Direction = rand() % 2;
    if (Direction == 0)
    {
        Angle = Tau32 - 0.5f*Theta + Angle;
    }
    else
    {
        Angle = Pi32 - 0.5f*Theta + Angle;
    }
    
    body *Body = GetBody(&State->Dynamics, State->Ball->BodyIndex);
    Body->F = V2(Cos(Angle), Sin(Angle)) * 30000.0f;
}


// Resume a that has already been started (after a player scored).
void Start(game_state *State)
{
    assert(State);
    
    State->Audio.Stop(State->Audio_Theme);
    
    ResetPositions(State);
    State->GameMode = GameMode_Playing;
    
    ServeBoll(State);
}


// Start a new game.
void NewGame(game_state *State)
{
    assert(State);
    
    State->Audio.Stop(State->Audio_Theme);
    
    ResetPositions(State);
    State->GameMode = GameMode_Playing;
    
    State->Scores[0] = 0;
    State->Scores[1] = 0;
    
    ServeBoll(State);
}


void Score(game_state *State, u32 ScoringPlayerIndex)
{
    State->Audio.Play(State->Audio_Score);
    
    State->GameMode = GameMode_Scored;
    ++State->Scores[ScoringPlayerIndex];
    ResetPositions(State);
}


void Pause(game_state *State)
{
    assert(State);
    
    State->GameMode = GameMode_Paused;
}


void End(game_state *State)
{
    assert(State);
    
    State->GameMode = GameMode_Inactive;
    State->Audio.Play(State->Audio_Theme);
}




//
// Rendering
//

void Render(game_state *State)
{
    assert(State);
    
    draw_calls *DrawCalls = &State->DrawCalls;
    
    
    //
    // @debug
    {
        v3 P0 = V3( 10.0f,  10.0f, 0.1f);
        v3 P1 = V3(810.0f, 110.0f, 0.1f);
        v3 P2 = V3(710.0f, 810.0f, 0.1f);
        PushPrimitiveTriangleOutline(&State->DrawCalls, P0, P1, P2, V4(1.0f, 0.0f, 0.0f, 1.0f));
        
        P0 = V3(210.0f, 410.0f, 0.1f);
        P1 = V3(510.0f, 450.0f, 0.1f);
        P2 = V3(310.0f, 510.0f, 0.1f);
        PushPrimitiveTriangleOutline(&State->DrawCalls, P0, P1, P2, V4(0.0f, 0.0f, 1.0f, 1.0f));
        
        P0 = V3(710.0f, 210.0f, 0.1f);
        P1 = V3(810.0f, 310.0f, 0.1f);
        PushPrimitiveRectangleOutline(&State->DrawCalls, P0, P1, V4(0.0f, 1.0f, 0.0f, 1.0f));
        
        P0 = V3(210.0f, 310.0f, 0.1f);
        P1 = V3(610.0f, 410.0f, 0.1f);
        PushPrimitiveRectangleFilled(&State->DrawCalls, P0, P1, V4(0.0f, 1.0f, 1.0f, 1.0f));
    }
    
    //
    // Background
    {
        v4 Colour = V4(0.6f, 0.6f, 0.8f, 1.0f);
        PushTexturedMesh(&State->DrawCalls, V3(0.0f, 0.0f, 0.9f), State->BackgroundMesh, State->BackgroundTexture, v2_one, Colour);
    }
    
    
    //
    // Entities
    {
        for (u32 Index = 0; Index < 5; ++Index)
        {
            entity *Entity = &State->Entities[Index];
            Render(&State->DrawCalls, Entity);
        }
    }
    
    
    f32 const Width  = (f32)DrawCalls->DisplayMetrics.WindowWidth;
    f32 const Height = (f32)DrawCalls->DisplayMetrics.WindowHeight;
    
    
    //
    // Scores
    wchar_t ScoreString[4];
    _snwprintf_s(ScoreString, 4, 3, L"%u", State->Scores[0]);
    PushText(DrawCalls, V2(0.5f * Width - 70.0f, 25.0f), ScoreString);
    
    _snwprintf_s(ScoreString, 4, 3, L"%u", State->Scores[1]);
    PushText(DrawCalls, V2(0.5f * Width + 70.0f, 25.0f), ScoreString);
    
    
    //
    // Text
    switch (State->GameMode)
    {
        case GameMode_Inactive:
        {
            f32  y = 200.0f;
            f32 dy =  70.0f;
            
            PushText(DrawCalls, V2(0.5f * Width, y)            , L"Pong!");
            PushText(DrawCalls, V2(0.5f * Width, y + dy)       , L"Press 0 to start a battle of the AI");
            PushText(DrawCalls, V2(0.5f * Width, y + 2.0f * dy), L"Press 1 to start a one player game");
            PushText(DrawCalls, V2(0.5f * Width, y + 3.0f * dy), L"Press 2 to start a two player game");
        } break;
        
        case GameMode_Paused:
        {
            PushText(DrawCalls, V2(0.5f * Width, 200.0f), L"Paused!");
        } break;
        
        case GameMode_Scored:
        {
            PushText(DrawCalls, V2(0.5f * Width, 200.0f), L"Score!");
            PushText(DrawCalls, V2(0.5f * Width, 235.0f), L"------");
            PushText(DrawCalls, V2(0.5f * Width, 290.0f), L"Press P to resume");
        } break;
    }
}




//
// Input
//

void ProcessInput(game_state *State)
{
    if (State->GameMode == GameMode_Inactive)
    {
        if (State->PressedKeys.count(0x30) > 0) // Key 0, AI vs AI
        {
            State->PressedKeys.erase(0x30);
            State->PlayerCount = 0;
            NewGame(State);
        }
        
        if (State->PressedKeys.count(0x31) > 0) // Key 1
        {
            State->PressedKeys.erase(0x31);
            State->PlayerCount = 1;
            NewGame(State);
        }
        
        if (State->PressedKeys.count(0x32) > 0) // Key 2
        {
            State->PressedKeys.erase(0x32);
            State->PlayerCount = 2;
            NewGame(State);
        }
    }
    else if (State->GameMode == GameMode_Scored)
    {
        if (State->PressedKeys.count(0x50) > 0 || State->PlayerCount == 0) // P
        {
            State->GameMode = GameMode_Playing;
            State->PressedKeys.erase(0x50);
            Start(State);
        }
    }
    else 
    {
        if (State->PressedKeys.count(0x50) > 0) // P
        {
            // Pause
            State->GameMode = State->GameMode == GameMode_Paused ? GameMode_Playing : GameMode_Paused;
            State->PressedKeys.erase(0x50);
        }
        
        if (State->PressedKeys.count(0x52) > 0) // R
        {
            // Reset
            State->PressedKeys.erase(0x52);
            ResetPositions(State);
            State->GameMode = GameMode_Inactive;
            State->Audio.Play(State->Audio_Theme);
        }
        
        if (State->GameMode == GameMode_Playing)
        {
            body *Body[2];
            Body[0] = GetBody(&State->Dynamics, State->Players[0]->BodyIndex);
            Body[1] = GetBody(&State->Dynamics, State->Players[1]->BodyIndex);
            
            Body[0]->F = v2_zero;
            Body[1]->F = v2_zero;
            f32 Force = 30000.0f;
            
            if (State->PressedKeys.count(0x57) > 0) // W
            {
                Body[0]->F += V2(0.0f, +Force);
            }
            
            if (State->PressedKeys.count(0x53) > 0) // S
            {
                Body[0]->F += V2(0.0f, -Force);
            }
            
            if (State->PressedKeys.count(0x26) > 0) // Arrow up
            {
                Body[1]->F += V2(0.0f, +Force);
            }
            
            if (State->PressedKeys.count(0x28) > 0) // Arrow down
            {
                Body[1]->F += V2(0.0f, -Force);
            }
        }
    }
}




//
// Misc.
// 

void ResetPositions(game_state *State)
{
    f32 const Width  = (f32)State->DrawCalls.DisplayMetrics.WindowWidth;
    f32 const Height = (f32)State->DrawCalls.DisplayMetrics.WindowHeight;
    
    v2 P = V2(0.5f * Width, 0.5f * Height);
    SetP(&State->Dynamics, State->Ball->BodyIndex, P);
    
    P = V2(1.5f * State->Players[1]->Size.x, 0.5f * Height);
    SetP(&State->Dynamics, State->Players[0]->BodyIndex, P);
    
    P = V2(Width - 1.5f * State->Players[1]->Size.x, 0.5f * Height);
    SetP(&State->Dynamics, State->Players[1]->BodyIndex, P);
    
    State->PressedKeys.clear();
}