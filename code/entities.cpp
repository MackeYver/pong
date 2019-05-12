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

#include "entities.h"
#include "draw_calls.h"
#include "game_main.h"

#define kVelocityLimit V2(1000.0f, 1000.0f)


//
// Forward declarations
//

void InitBall(game_state *State, entity *Entity, mesh_index MIndex);
void InitPaddles(game_state *State, entity *LeftPadde, entity *RightPaddle, mesh_index MIndex);
void InitWalls(game_state *State, entity *TopWall, entity *BottomWall);



//
// General entity functions
//

void InitEntities(game_state *State)
{
    //
    // Create 1x1 quad with UVs
    f32 const w = 0.5f;
    f32 const h = 0.5f;
    
    v3 P[] =
    {
        {-w, +h, 0.0f},
        {-w, -h, 0.0f},
        {+w, -h, 0.0f},
        
        {-w, +h, 0.0f},
        {+w, -h, 0.0f},
        {+w, +h, 0.0f},
    };
    
    f32 s = 1.0f;
    f32 t = 1.0f;
    
    v2 UV[] =
    {
        {0.0f, 0.0f},
        {0.0f,    t},
        {   s,    t},
        
        {0.0f, 0.0f},
        {   s,    t},
        {   s, 0.0f},
    };
    
    mesh_index MIndex = LoadMesh(&State->Resources, P, UV, 6);
    assert(MIndex >= 0);
    
    
    //
    // Init the entities
    State->Ball = &State->Entities[0];
    State->Players[0] = &State->Entities[1];
    State->Players[1] = &State->Entities[2];
    State->Walls[0] = &State->Entities[3];
    State->Walls[1] = &State->Entities[4];
    
    InitPaddles(State, State->Players[0], State->Players[1], MIndex);
    InitBall(State, State->Ball, MIndex);
    InitWalls(State, State->Walls[0], State->Walls[1]);
}

void Update(dynamics_state *State, entity *Entity)
{
    body *Body = GetBody(State, Entity->BodyIndex);
    Entity->P = Body->P;
}


void Render(draw_calls *DrawCalls, entity *Entity)
{
    PushTexturedMesh(DrawCalls, V3(Entity->P, 0.0f), Entity->MeshIndex, Entity->TextureIndex, Entity->Scale);
}




//
// Ball
//

void InitBall(game_state *State, entity *Entity, mesh_index MIndex)
{
    v2 Size = V2(25.0f, 25.0f);
    f32 BallRadius = 25.0f;
    f32 BallArea = Pi32 * BallRadius * BallRadius;
    f32 BallDensity = 0.0004f;
    
    body *Body = NewCircleBody(&State->Dynamics, 25.0f / 2.0f, Entity);
    Body->dPMax = kVelocityLimit;
    Body->dPMask = v2_one;
    Body->Damping = 0.0f;
    Body->InverseMass = 1.0f / (BallArea * BallDensity);
    
    Entity->Colour = v4_one;
    Entity->Size = Size;
    Entity->Scale = Size;
    Entity->BodyIndex = GetBodyIndex(&State->Dynamics, Body);
    Entity->MeshIndex = MIndex;
    Entity->Audio_BallBounce = -1; // The ball can't bounce on itself...
    
    //
    // Texture
    texture_index TextureIndex = LoadBMP(&State->Resources, "data\\ball.bmp");
    assert(TextureIndex >= 0);
    Entity->TextureIndex = TextureIndex;
}


void InitPaddles(game_state *State, entity *LeftPaddle, entity *RightPaddle, mesh_index MIndex)
{
    //
    // Texture
    texture_index TextureIndex = LoadBMP(&State->Resources, "data\\paddle.bmp");
    assert(TextureIndex >= 0);
    
    
    //
    // Audio
    voice_index VoiceIndex = LoadWAV(&State->Resources, "data\\paddle_bounce.wav");
    assert(VoiceIndex >= 0);
    
    //
    // Size and area
    v2 Size = V2(25.0f, 140.0f);
    f32 Area = Size.x * Size.y;
    f32 Density = 0.0005f;
    f32 InverseMass = 1.0f / (Area * Density);
    
    
    //
    // Paddles
    for (u32 Index = 0; Index < 2; ++Index)
    {
        entity *Entity = State->Players[Index];
        body *Body = NewRectangleBody(&State->Dynamics, Size, Entity);
        Body->dPMax = 2.0f * kVelocityLimit;
        Body->dPMask = V2(0.0f, 1.0f);
        Body->Damping = 0.5f;
        Body->InverseMass = InverseMass;
        
        Entity->Colour = V4(1.0f, 0.0f, 0.0f, 1.0f);
        Entity->BodyIndex = GetBodyIndex(&State->Dynamics, Body);
        Entity->Size = Size;
        Entity->Scale = Size;
        Entity->MeshIndex = MIndex;
        Entity->TextureIndex = TextureIndex;
        Entity->Audio_BallBounce = VoiceIndex;
    }
}


void InitWalls(game_state *State, entity *TopWall, entity *BottomWall)
{
    //
    // Sizes and stuff
    f32 const Width  = (f32)State->DrawCalls.DisplayMetrics.WindowWidth;
    f32 const Height = (f32)State->DrawCalls.DisplayMetrics.WindowHeight;
    v2  const Size   = V2(Width, 0.05f * Height);
    
    f32 const MidX = 0.5f * Width;
    f32 const TopY = (f32)Height - 0.5f * Size.y;
    f32 const BottomY = 0.5f * Size.y;
    
    
    //
    // Mesh and texture
    f32 const w = 0.5f * Size.x;
    f32 const h = 0.5f * Size.y;
    
    v3 P[] =
    {
        {-w, +h, 0.0f},
        {-w, -h, 0.0f},
        {+w, -h, 0.0f},
        
        {-w, +h, 0.0f},
        {+w, -h, 0.0f},
        {+w, +h, 0.0f},
    };
    
    f32 s = Size.x / 128.0f;
    f32 t = 0.29;
    
    v2 UV[] =
    {
        {0.0f, 0.0f},
        {0.0f,    t},
        {   s,    t},
        
        {0.0f, 0.0f},
        {   s,    t},
        {   s, 0.0f},
    };
    
    mesh_index MeshIndex = LoadMesh(&State->Resources, P, UV, 6);
    assert(MeshIndex >= 0);
    
    texture_index TextureIndex = LoadBMP(&State->Resources, "data\\brick.bmp");
    assert(TextureIndex >= 0);
    
    
    //
    // Audio
    voice_index VoiceIndex = LoadWAV(&State->Resources, "data\\wall_bounce.wav");
    assert(VoiceIndex >= 0);
    
    
    //
    // Top wall
    body *Body = NewRectangleBody(&State->Dynamics, Size, TopWall);
    Body->P = V2(MidX, TopY);
    Body->InverseMass = 0.0f;
    Body->dPMask = v2_zero;
    
    TopWall->MeshIndex = MeshIndex;
    TopWall->TextureIndex = TextureIndex;
    TopWall->Audio_BallBounce = VoiceIndex;
    TopWall->Colour = v4_one;
    TopWall->BodyIndex = GetBodyIndex(&State->Dynamics, Body);
    TopWall->Size = Size;
    TopWall->Scale = v2_one;
    
    
    //
    // Bottom wall
    Body = NewRectangleBody(&State->Dynamics, Size, BottomWall);
    Body->P = V2(MidX, BottomY);;
    Body->InverseMass = 0.0f;
    Body->dPMask = v2_zero;
    
    BottomWall->MeshIndex = MeshIndex;
    BottomWall->TextureIndex = TextureIndex;
    BottomWall->Audio_BallBounce = VoiceIndex;
    BottomWall->Colour = v4_one;
    BottomWall->BodyIndex = GetBodyIndex(&State->Dynamics, Body);
    BottomWall->Size = Size;
    BottomWall->Scale = v2_one;
}