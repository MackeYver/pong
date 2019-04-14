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

void Reset(game_state *State);
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
    
    //
    // Load audio
    {
        char const *PathAndFilename[] = 
        {
            "data\\theme.wav",
            "data\\paddle_bounce.wav",
            "data\\border_bounce.wav",
            "data\\score.wav",
        };
        
        for (u32 Index = 0; Index < AudioVoice_Count; ++Index)
        {
            voice_index VIndex = LoadWAV(&State->Resources, PathAndFilename[Index]);
            assert(VIndex >= 0);
            State->AudioIndices[Index] = VIndex;
        }
        
        State->Audio.Play(AudioVoice_Theme);
    }
    
    
    //
    // Load textures
    {
        // TODO(Marcus): Implement!
    }
    
    
    //
    // Game state
    State->BackgroundColour = V4(0.0f, 0.0f, 1.0f, 1.0f); // TODO(Marcus): currently unused!
    State->PlayerCount = 0;
    
    
    //
    // Setup entities (hardcoded for now)
    {
        
        //
        // Players
        {
            v2 Size = V2(25.0f, 140.0f);
            v2 SpeedLimit = V2(1000.0f, 1000.0f);
            
            f32 PaddleArea = Size.x * Size.y;
            f32 PaddleDensity = 0.0005f;
            f32 PaddleInverseMass = 1.0f / (PaddleArea * PaddleDensity);
            
            // Player0
            State->Players[0].BodyIndex = NewRectangleBody(&State->Dynamics, Size, &State->Players[0]);
            State->Players[0].Colour = V4(1.0f, 0.0f, 0.0f, 1.0f);
            body *Body = GetBody(&State->Dynamics, State->Players[0].BodyIndex);
            Body->dPMax = 2.0f*SpeedLimit;
            Body->dPMask = V2(0.0f, 1.0f);
            Body->Damping = 0.5f;
            Body->InverseMass = PaddleInverseMass;
            
            // Player1
            State->Players[1].BodyIndex = NewRectangleBody(&State->Dynamics, Size, &State->Players[1]);
            State->Players[1].Colour = V4(0.0f, 1.0f, 0.0f, 1.0f);
            Body = GetBody(&State->Dynamics, State->Players[1].BodyIndex);
            Body->dPMax = 2.0f*SpeedLimit;
            Body->dPMask = V2(0.0f, 1.0f);
            Body->Damping = 0.5f;
            Body->InverseMass = PaddleInverseMass;
            
            
            //
            // Ball
            f32 BallRadius = 25.0f;
            f32 BallArea = Pi32 * BallRadius * BallRadius;
            f32 BallDensity = 0.0004f;
            
            State->Ball.BodyIndex = NewRectangleBody(&State->Dynamics, V2(25.0f, 25.0f), &State->Ball);
            State->Ball.Colour = V4(1.0f, 1.0f, 1.0f, 1.0f);
            
            Body = GetBody(&State->Dynamics, State->Ball.BodyIndex);
            Body->dPMax = SpeedLimit;
            Body->dPMask = v2_one;
            Body->Damping = 0.0f;
            Body->InverseMass = 1.0f / (BallArea * BallDensity);
            
            Reset(State); // Resets the dynamic state (P, dP, etc..)
        }
        
        
        //
        // Borders
        {
            f32 const Width  = (f32)State->DrawCalls.DisplayMetrics.WindowWidth;
            f32 const Height = (f32)State->DrawCalls.DisplayMetrics.WindowHeight;
            
            f32 const vh = 8.0f;  // Visible height
            f32 const h = 160.0f; // Actual height (larger due to sloppy collision detection)
            v2 Size = V2(2.0f*Width, h);
            
            f32 const MidX = 0.5f * Width;
            f32 const TopY = (f32)Height - vh + 0.5f*h;
            f32 const BottomY = vh - 0.5f*h;
            
            //
            // Bottom border
            State->Borders[0].Colour = V4(0.0f, 0.0f, 1.0f, 1.0f);
            State->Borders[0].BodyIndex = NewRectangleBody(&State->Dynamics, Size, &State->Borders[0]);
            body *Border = GetBody(&State->Dynamics, State->Borders[0].BodyIndex);
            Border->P = V2(MidX, BottomY);
            Border->InverseMass = 0.0f;
            Border->dPMask = v2_zero;
            
            //
            // Top border
            State->Borders[1].Colour = V4(0.0f, 0.0f, 1.0f, 1.0f);
            State->Borders[1].BodyIndex = NewRectangleBody(&State->Dynamics, Size, &State->Borders[1]);
            Border = GetBody(&State->Dynamics, State->Borders[1].BodyIndex);
            Border->P = V2(MidX, TopY);
            Border->InverseMass = 0.0f;
            Border->dPMask = v2_zero;
        }
    }
    
    
    //
    // Seed the RNG
    {
        time_t Time = time(nullptr) * time(nullptr);
        srand(static_cast<unsigned int>(Time));
    }
}


void Update(game_state *State, f32 dt)
{
    assert(State);
    assert(dt > 0.0f);
    
    
    //
    // Process inputs
    ProcessInput(State);
    
    
    //
    // "Physics"
    if (State->GameMode == GameMode_Playing)
    {
        Update(&State->Dynamics, dt);
        
        std::vector<collision_info> static Collisions;
        Collisions.clear();
        
        DetectCollisions(&State->Dynamics, Collisions);
        
        for (auto& Collision : Collisions)
        {
            //
            // Will be used in order to determine if we shall play any sounds
            b32 TheBallIsInvolved = false;
            b32 ABorderIsInvolved = false;
            b32 APlayerIsInvolved = false;
            
            
            //
            // Check what entities were involved in the collision
            if ((Collision.BodiesInvolved[0] == State->Ball.BodyIndex) || (Collision.BodiesInvolved[1] == State->Ball.BodyIndex))
            {
                TheBallIsInvolved = true;
            }
            
            for (u32 Index = 0; Index < 2; ++Index)
            {
                if ((Collision.BodiesInvolved[0] == State->Players[Index].BodyIndex) ||
                    (Collision.BodiesInvolved[1] == State->Players[Index].BodyIndex))
                {
                    APlayerIsInvolved = true;
                }
                
                if ((Collision.BodiesInvolved[0] == State->Borders[Index].BodyIndex) ||
                    (Collision.BodiesInvolved[1] == State->Borders[Index].BodyIndex))
                {
                    ABorderIsInvolved = true;
                }
            }
            
            
            //
            // Play sounds
            if (TheBallIsInvolved && APlayerIsInvolved)
            {
                State->Audio.Play(AudioVoice_PaddleBounce);
                Collision.ForceModifier = 0.25f;
            }
            else if (TheBallIsInvolved && ABorderIsInvolved)
            {
                State->Audio.Play(AudioVoice_BorderBounce);
                Collision.ForceModifier = -0.15f;
            }
            else if (APlayerIsInvolved && ABorderIsInvolved)
            {
                Collision.SkipForceApplication = true;
            }
            
        }
        
        ResolveCollisions(&State->Dynamics, Collisions, dt);
    }
    
    
    //
    // Check if any of the players scored.
    {
        body *Body = GetBody(&State->Dynamics, State->Ball.BodyIndex);
        if (Body->P.x < 0.0f)
        {
            Score(State, 1);
        }
        else if (Body->P.x > State->DrawCalls.DisplayMetrics.WindowWidth)
        {
            Score(State, 0);
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
    
    body *Body = GetBody(&State->Dynamics, State->Ball.BodyIndex);
    Body->F = V2(Cos(Angle), Sin(Angle)) * 30000.0f;
}


// Resume a that has already been started (after a player scored).
void Start(game_state *State)
{
    assert(State);
    
    State->Audio.Stop(AudioVoice_Theme);
    
    Reset(State);
    State->GameMode = GameMode_Playing;
    
    ServeBoll(State);
}


// Start a new game.
void NewGame(game_state *State)
{
    assert(State);
    
    State->Audio.Stop(AudioVoice_Theme);
    
    Reset(State);
    State->GameMode = GameMode_Playing;
    
    State->Players[0].Score = 0;
    State->Players[1].Score = 0;
    
    ServeBoll(State);
}


void Score(game_state *State, u32 ScoringPlayerIndex)
{
    State->Audio.Play(AudioVoice_Score);
    
    State->GameMode = GameMode_Scored;
    ++State->Players[ScoringPlayerIndex].Score;
    Reset(State);
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
    State->Audio.Play(AudioVoice_Theme);
}




//
// Rendering
//

void RenderBodyAsRectangle(draw_calls *DrawCalls, body *Body, v4 Colour)
{
    PushFilledRectangle(DrawCalls, Body->P, 2.0f*Body->Shape.HalfSize, Colour);
}


void RenderBodyAsCircle(draw_calls *DrawCalls, body *Body, v4 Colour)
{
    PushFilledCircle(DrawCalls, Body->P, Body->Shape.Radius, Colour);
}


void RenderScores(game_state *State)
{
    draw_calls *DrawCalls = &State->DrawCalls;
    
    f32 const Width  = (f32)DrawCalls->DisplayMetrics.WindowWidth;
    f32 const Height = (f32)DrawCalls->DisplayMetrics.WindowHeight;
    
    //
    // Scores
    wchar_t ScoreString[4];
    _snwprintf_s(ScoreString, 4, 3, L"%u", State->Players[0].Score);
    PushText(DrawCalls, V2(0.5f * Width - 70.0f, 48.0f), ScoreString);
    
    _snwprintf_s(ScoreString, 4, 3, L"%u", State->Players[1].Score);
    PushText(DrawCalls, V2(0.5f * Width + 70.0f, 48.0f), ScoreString);
}


void Render(game_state *State)
{
    assert(State);
    
    draw_calls *DrawCalls = &State->DrawCalls;
    
    f32 const Width  = (f32)DrawCalls->DisplayMetrics.WindowWidth;
    f32 const Height = (f32)DrawCalls->DisplayMetrics.WindowHeight;
    
    //
    // Render
    {
        RenderBodyAsRectangle(DrawCalls, GetBody(&State->Dynamics, State->Players[0].BodyIndex), State->Players[0].Colour);
        RenderBodyAsRectangle(DrawCalls, GetBody(&State->Dynamics, State->Players[1].BodyIndex), State->Players[1].Colour);
        
        RenderBodyAsRectangle(DrawCalls, GetBody(&State->Dynamics, State->Borders[0].BodyIndex), State->Borders[0].Colour);
        RenderBodyAsRectangle(DrawCalls, GetBody(&State->Dynamics, State->Borders[1].BodyIndex), State->Borders[1].Colour);
        
        RenderBodyAsCircle(DrawCalls, GetBody(&State->Dynamics, State->Ball.BodyIndex), State->Ball.Colour);
    }
    
    RenderScores(State);
    
    switch (State->GameMode)
    {
        case GameMode_Inactive:
        {
            f32  y = 200.0f;
            f32 dy =  70.0f;
            
            PushText(DrawCalls, V2(0.5f * Width, y)            , L"Pong!");
            PushText(DrawCalls, V2(0.5f * Width, y + dy)       , L"Press 1 to start a one player game");
            PushText(DrawCalls, V2(0.5f * Width, y + 2.0f * dy), L"Press 2 to start a two player game");
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
        State->PlayerCount = 0;
        
        if (State->PressedKeys.count(0x31) > 0) // Key 1
        {
            State->PressedKeys.erase(0x31);
            State->PlayerCount = 1;
        }
        
        if (State->PressedKeys.count(0x32) > 0) // Key 2
        {
            State->PressedKeys.erase(0x32);
            State->PlayerCount = 2;
        }
        
        if (State->PlayerCount > 0)
        {
            NewGame(State);
        }
    }
    else if (State->GameMode == GameMode_Scored)
    {
        if (State->PressedKeys.count(0x50) > 0) // P
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
            Reset(State);
            State->GameMode = GameMode_Inactive;
            State->Audio.Play(AudioVoice_Theme);
        }
        
        if (State->GameMode == GameMode_Playing)
        {
            b32 Up1 = false;
            b32 Down1 = false;
            
            b32 Up2 = false;
            b32 Down2 = false;
            
            if (State->PressedKeys.count(0x57) > 0) // W
            {
                Up1 = true;
            }
            
            if (State->PressedKeys.count(0x53) > 0) // S
            {
                Down1 = true;
            }
            
            if (State->PressedKeys.count(0x26) > 0) // Arrow up
            {
                Up2 = true;
            }
            
            if (State->PressedKeys.count(0x28) > 0) // Arrow down
            {
                Down2 = true;
            }
            
            //
            // Apply forces
            body *Body[2];
            Body[0] = GetBody(&State->Dynamics, State->Players[0].BodyIndex);
            Body[1] = GetBody(&State->Dynamics, State->Players[1].BodyIndex);
            
            Body[0]->F = v2_zero;
            Body[1]->F = v2_zero;
            f32 Force = 30000.0f;
            
            if (Up1 || (Up2 && State->PlayerCount == 1))
            {
                Body[0]->F += V2(0.0f, +Force);
            }
            
            if (Down1 || (Down2 && State->PlayerCount == 1))
            {
                Body[0]->F += V2(0.0f, -Force);
            }
            
            if (Up2 && State->PlayerCount == 2)
            {
                Body[1]->F += V2(0.0f, +Force);
            }
            
            if (Down2 && State->PlayerCount == 2)
            {
                Body[1]->F += V2(0.0f, -Force);
            }
        }
    }
}




//
// Misc.
// 

void Reset(game_state *State)
{
    f32 const Width  = (f32)State->DrawCalls.DisplayMetrics.WindowWidth;
    f32 const Height = (f32)State->DrawCalls.DisplayMetrics.WindowHeight;
    
    
    //
    // Ball
    body *Body = GetBody(&State->Dynamics, State->Ball.BodyIndex);
    Body->P = V2(0.5f * Width, 0.5f * Height);
    Body->PrevP = Body->P;
    
    Body->dP = v2_zero;
    Body->F = v2_zero;
    
    
    //
    // Paddles
    v2 Size = V2(25.0f, 140.0f);
    v2 Po[2] = 
    {
        V2(2.5f * Size.x, 0.5f * Height),
        V2(Width - 2.5f * Size.x, 0.5f * Height)
    };
    
    for (int Index = 0; Index < 2; ++Index)
    {
        Body = GetBody(&State->Dynamics, State->Players[Index].BodyIndex);
        
        Body->P = Po[Index];
        Body->PrevP = Po[Index];
        
        Body->dP = v2_zero;
        Body->F = v2_zero;
    }
}