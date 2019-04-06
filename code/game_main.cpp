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
#include <vector>

#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif




//
// Forward declarations
//

void Render(game_state *State);
void UpdateAllBodies(game_state *State, f32 dt);
void ProcessInput(game_state *State);

void NewGame(game_state *State);
void Start(game_state *State);
void Pause(game_state *State);
void Score(game_state *State, u32 ScoringPlayerIndex);
void End(game_state *State);




//
// Misc. (will be moved)
// TODO(Marcus): Move the functins below...
// 

u32 NewRectangleBody(game_state *State, v2 Size)
{
    assert(State);
    
    u32 Index = State->Bodies.size();
    State->Bodies.emplace_back(body());
    body *Result = &State->Bodies.back();
    
    Result->Shape.Type = ShapeType_rectangle;
    Result->Shape.HalfSize = 0.5f*Size;
    Result->Shape.BodyIndex = Index;
    
    return Index;
}

u32 NewCircleBody(game_state *State, f32 Radius)
{
    assert(State);
    
    u32 Index = State->Bodies.size();
    State->Bodies.emplace_back(body());
    body *Result = &State->Bodies.back();
    
    Result->Shape.Type = ShapeType_circle;
    Result->Shape.Radius = Radius;
    Result->Shape.BodyIndex = Index;
    
    return Index;
}


void Reset(game_state *State)
{
    static const f32 Width  = (f32)State->Renderer->DisplayMetrics.WindowWidth;
    static const f32 Height = (f32)State->Renderer->DisplayMetrics.WindowHeight;
    
    
    //
    // Ball
    v2 SpeedLimit = V2(600.0f, 600.0f);
    
    f32 BallRadius = 25.0f;
    f32 BallArea = Pi32 * BallRadius * BallRadius;
    f32 BallDensity = 0.0004f;
    
    body *Body = &State->Bodies[State->Ball.BodyIndex];
    Body->P = V2(0.5f * Width, 0.5f * Height);
    Body->PrevP = Body->P;
    
    Body->dP = v2_zero;
    Body->dPMax = SpeedLimit;
    Body->dPMask = v2_one;
    
    Body->F = v2_zero;
    Body->Damping = 0.0f;
    Body->InverseMass = 1.0f / (BallArea * BallDensity);
    
    
    //
    // Paddles
    v2 PaddleSize = V2(20.0f, 140.0f);
    
    f32 PaddleArea = PaddleSize.x * PaddleSize.y;
    f32 PaddleDensity = 0.0005f;
    f32 InverseMass = 1.0f / (PaddleArea * PaddleDensity);
    
    v2 Po[2] = 
    {
        V2(2.5f * PaddleSize.x, 0.5f * Height),
        V2(Width - 2.5f * PaddleSize.x, 0.5f * Height)
    };
    
    for (int Index = 0; Index < 2; ++Index)
    {
        Body = &State->Bodies[State->Players[Index].BodyIndex];
        
        Body->P = Po[Index];
        Body->PrevP = Po[Index];
        
        Body->dP = v2_zero;
        Body->dPMax = 2.0f*SpeedLimit;
        Body->dPMask = V2(0.0f, 1.0f);
        
        Body->F = v2_zero;
        Body->Damping = 0.5f;
        Body->InverseMass = InverseMass;
    }
}




//
// Init
//

void Init(game_state *State)
{
    assert(State);
    assert(State->Renderer);
    
    
    //
    // Game state
    State->BackgroundColour = V4(0.0f, 0.0f, 1.0f, 1.0f);
    State->PlayerCount = 0;
    
    
    //
    // Static state
    {
        v2 Size = V2(30.0f, 140.0f);
        
        State->Players[0].BodyIndex = NewRectangleBody(State, Size);
        State->Players[0].Colour = V4(1.0f, 0.0f, 0.0f, 1.0f);
        
        State->Players[1].BodyIndex = NewRectangleBody(State, Size);
        State->Players[1].Colour = V4(0.0f, 1.0f, 0.0f, 1.0f);
        
        State->Ball.BodyIndex = NewRectangleBody(State, V2(25.0f, 25.0f));//NewCircleBody(State, 25.0f);
        State->Ball.Colour = V4(1.0f, 1.0f, 1.0f, 1.0f);
        
        Reset(State); // Resets the dynamic state (P, dP, etc..)
    }
    
    
    //
    // Borders
    {
        static const f32 Width  = (f32)State->Renderer->DisplayMetrics.WindowWidth;
        static const f32 Height = (f32)State->Renderer->DisplayMetrics.WindowHeight;
        
        f32 const vh = 8.0f;  // Visible height
        f32 const h = 160.0f; // Actual height (larger due to sloppy collision detection)
        v2 Size = V2(2.0f*Width, h);
        
        f32 const MidX = 0.5f * Width;
        f32 const TopY = (f32)Height - vh + 0.5f*h;
        f32 const BottomY = vh - 0.5f*h;
        
        //
        // Bottom border
        State->Borders[0].Colour = V4(0.0f, 0.0f, 1.0f, 1.0f);
        State->Borders[0].BodyIndex = NewRectangleBody(State, Size);
        body *Border = &State->Bodies[State->Borders[0].BodyIndex];
        Border->P = V2(MidX, BottomY);
        Border->InverseMass = 0.0f;
        Border->dPMask = v2_zero;
        
        //
        // Top border
        State->Borders[1].Colour = V4(0.0f, 0.0f, 1.0f, 1.0f);
        State->Borders[1].BodyIndex = NewRectangleBody(State, Size);
        Border = &State->Bodies[State->Borders[1].BodyIndex];
        Border->P = V2(MidX, TopY);
        Border->InverseMass = 0.0f;
        Border->dPMask = v2_zero;
    }
    
    
    //
    // Seed the RNG
    {
        time_t Time = time(nullptr) * time(nullptr);
        srand(static_cast<unsigned int>(Time));
    }
}




//
// Shutdown
//

void Shutdown(game_state *State)
{
    assert(State);
}




//
// Update
//

void Update(game_state *State, f32 dt)
{
    assert(State);
    assert(dt > 0.0f);
    assert(State->Renderer);
    
    //
    // Process inputs
    ProcessInput(State);
    
    
    //
    // "Physics"
    if (State->GameMode == GameMode_Playing)
    {
        UpdateAllBodies(State, dt);
    }
    
    
    //
    // Check if any of the players scored.
    {
        body *Body = &State->Bodies[State->Ball.BodyIndex];
        if (Body->P.x < 0.0f)
        {
            Score(State, 1);
        }
        else if (Body->P.x > State->Renderer->DisplayMetrics.WindowWidth)
        {
            Score(State, 0);
        }
    }
    
    
    //
    // Rendering
    Render(State);
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
    
    body *Body = &State->Bodies[State->Ball.BodyIndex];
    Body->F = V2(Cos(Angle), Sin(Angle)) * 30000.0f;
}


void Start(game_state *State)
{
    assert(State);
    
    Reset(State);
    State->GameMode = GameMode_Playing;
    
    ServeBoll(State);
}


void NewGame(game_state *State)
{
    assert(State);
    
    Reset(State);
    State->GameMode = GameMode_Playing;
    
    State->Players[0].Score = 0;
    State->Players[1].Score = 0;
    
    ServeBoll(State);
}


void Score(game_state *State, u32 ScoringPlayerIndex)
{
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
}




//
// Rendering misc.
//

void RenderBodyAsRectangle(renderer *Renderer, body *Body, v4 Colour)
{
    PushFilledRectangle(Renderer, Body->P, 2.0f*Body->Shape.HalfSize, Colour);
}

void RenderBodyAsCircle(renderer *Renderer, body *Body, v4 Colour)
{
    PushFilledRectangle(Renderer, Body->P, 2.0f*Body->Shape.HalfSize, Colour);
}


void RenderScores(game_state *State)
{
    renderer *Renderer = State->Renderer;
    
    f32 const Width  = (f32)Renderer->DisplayMetrics.WindowWidth;
    f32 const Height = (f32)Renderer->DisplayMetrics.WindowHeight;
    
    //
    // Scores
    wchar_t ScoreString[4];
    _snwprintf_s(ScoreString, 4, 3, L"%u", State->Players[0].Score);
    PushText(State->Renderer, V2(0.5f * Width - 70.0f, 48.0f), ScoreString);
    
    _snwprintf_s(ScoreString, 4, 3, L"%u", State->Players[1].Score);
    PushText(State->Renderer, V2(0.5f * Width + 70.0f, 48.0f), ScoreString);
}


void Render(game_state *State)
{
    assert(State);
    assert(State->Renderer);
    
    renderer *Renderer = State->Renderer;
    
    f32 const Width  = (f32)Renderer->DisplayMetrics.WindowWidth;
    f32 const Height = (f32)Renderer->DisplayMetrics.WindowHeight;
    
    //
    // Render
    {
        RenderBodyAsRectangle(Renderer, &State->Bodies[State->Players[0].BodyIndex], State->Players[0].Colour);
        RenderBodyAsRectangle(Renderer, &State->Bodies[State->Players[1].BodyIndex], State->Players[1].Colour);
        
        RenderBodyAsRectangle(Renderer, &State->Bodies[State->Borders[0].BodyIndex], State->Borders[0].Colour);
        RenderBodyAsRectangle(Renderer, &State->Bodies[State->Borders[1].BodyIndex], State->Borders[1].Colour);
        
        RenderBodyAsCircle(Renderer, &State->Bodies[State->Ball.BodyIndex], State->Ball.Colour);
    }
    
    RenderScores(State);
    
    switch (State->GameMode)
    {
        case GameMode_Inactive:
        {
            f32  y = 200.0f;
            f32 dy =  70.0f;
            
            PushText(State->Renderer, V2(0.5f * Width, y)            , L"Pong!");
            PushText(State->Renderer, V2(0.5f * Width, y + dy)       , L"Press 1 to start a one player game");
            PushText(State->Renderer, V2(0.5f * Width, y + 2.0f * dy), L"Press 2 to start a two player game");
        } break;
        
        case GameMode_Paused:
        {
            PushText(State->Renderer, V2(0.5f * Width, 200.0f), L"Paused!");
        } break;
        
        case GameMode_Scored:
        {
            PushText(State->Renderer, V2(0.5f * Width, 200.0f), L"Score!");
            PushText(State->Renderer, V2(0.5f * Width, 235.0f), L"------");
            
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
            printf("1 pressed: %d!\n", State->PressedKeys.count(0x31));
            State->PressedKeys.erase(0x31);
            State->PlayerCount = 1;
        }
        
        if (State->PressedKeys.count(0x32) > 0) // Key 2
        {
            printf("2 pressed: %d!\n", State->PressedKeys.count(0x32));
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
        if (State->PressedKeys.size() > 0)
        {
            State->GameMode = GameMode_Playing;
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
            Body[0] = &State->Bodies[State->Players[0].BodyIndex];
            Body[1] = &State->Bodies[State->Players[1].BodyIndex];
            
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
// "Physics"
//


// Direction should be of unit length
v2 FurthestPointInRectangle(v2 Po, shape *Rectangle, v2 Direction)
{
    assert(Rectangle);
    
    v2 H = Rectangle->HalfSize;
    v2 dX = V2(2.0f*H.x, 0.0f);
    v2 dY = V2(0.0f    , 2.0f*H.y);
    
    v2 Vertices[4] = {
        {Po + H},
        {Po + H - dX},
        {Po - H},
        {Po - H + dX},
    }; 
    
    f32 Distance = -(f32Max - 1);
    v2 Result = v2_zero;
    
    for (u32 Index = 0; Index < 4; ++Index)
    {
        f32 CurrDistance = Dot(Vertices[Index], Direction);
        if (CurrDistance > Distance)
        {
            Distance = CurrDistance;
            Result = Vertices[Index];
        }
    }
    
    return Result;
}


// Direction should be of unit length
v2 FurthestPointInCircle(v2 Po, shape *Circle, v2 Direction)
{
    v2 Result = Po + Circle->Radius * Direction;
    return Result;
}


v2 FurthestPointInShape(v2 Po, shape *Shape, v2 Direction)
{
    assert(Shape);
    
    v2 Result;
    
    switch (Shape->Type)
    {
        case ShapeType_rectangle: 
        {
            Result = FurthestPointInRectangle(Po, Shape, Direction);
        } break;
        
        case ShapeType_circle:
        {
            Result = FurthestPointInCircle(Po, Shape, Direction);
        } break;
        
        default:
        {
            Result = v2_zero;
            assert(0);
        } break;
    }
    
    return Result;
}


v2 NormalTowards(v2 Line, v2 Direction)
{
    v2 Perp0 = V2( Line.y, -Line.x);
    v2 Perp1 = V2(-Line.y,  Line.x);
    
    f32 dot0 = Dot(Perp0, Direction);
    f32 dot1 = Dot(Perp1, Direction);
    
    v2 Result = dot0 > dot1 ? Perp0 : Perp1;
    return Normalize(Result);
}


b32 ContainsOrigin(std::vector<v2>& Simplex, v2 *Direction)
{
    b32 Result = false;
    
    v2 A = Simplex.back();
    v2 AO = Normalize(-A);
    
    if (Simplex.size() == 3)
    {
        // Triangle!
        v2 B = Simplex[1];
        v2 C = Simplex[0];
        
        v2 AB = B - A;
        v2 AC = C - A;
        
        v2 ABPerp = -NormalTowards(AB, AC); // The normal that points away from C
        v2 ACPerp = -NormalTowards(AC, AB); // The normal that points awaw from B
        
        if (Dot(ABPerp, AO) > 0)
        {
            // The origin is outside and in the direction of AB, remove point C
            Simplex.erase(Simplex.begin());
            *Direction = ABPerp;
        }
        else if (Dot(ACPerp, AO) > 0)
        {
            // The origin is outside and in the direction of AC, remove point B
            Simplex.erase(Simplex.begin() + 1);
            *Direction = ACPerp;
        }
        else
        {
            // Origin is in the triangle!
            Result = true;
        }
    }
    else if (Simplex.size() == 2)
    {
        // We are a line! Yay, let's go do some typical line-things.
        v2 B = Simplex[0];
        v2 AB = B - A;
        
        v2 Normal = NormalTowards(AB, AO);
        *Direction = Normal;
    }
    else
    {
        assert(0); // Invalid path!
    }
    
    return Result;
}


struct edge
{
    v2 N;
    f32 Distance;
    u32 VertexIndex;
};


edge FindEdgeClosestToOrigin(std::vector<v2>& Simplex)
{
    edge Result;
    Result.Distance = f32Max;
    
    std::vector<v2>::size_type Size = Simplex.size();
    for (u32 CurrIndex = 0; CurrIndex < Size; ++CurrIndex)
    {
        u32 NextIndex = (CurrIndex + 1) % Size;
        
        v2 A = Simplex[CurrIndex];
        v2 B = Simplex[NextIndex];
        v2 E = B - A;
        v2 N = NormalTowards(E, A);
        
        f32 CurrDistance = Dot(N, A);
        if (CurrDistance < Result.Distance)
        {
            Result.Distance = CurrDistance;
            Result.N = N;
            Result.VertexIndex = NextIndex;
        }
    }
    
    return Result;
}


struct collision_info
{
    v2 N;
    f32 Depth;
};


collision_info GetCollisionInfo(std::vector<v2>& Simplex, v2 Poa, shape *A, v2 Pob, shape *B)
{
    while (1) {
        edge Edge = FindEdgeClosestToOrigin(Simplex);
        
        v2 P = FurthestPointInShape(Poa, A, Edge.N) - FurthestPointInShape(Pob, B, -Edge.N);
        
        f32 Distance = Dot(P, Edge.N);
        if (Distance - Edge.Distance < 0.00001f) {
            collision_info Result;
            Result.N = Edge.N;
            Result.Depth = Distance;
            return Result;
        } else {
            Simplex.insert(Simplex.begin() + Edge.VertexIndex, P);
        }
    }
}


b32 Intersects(body *A, body *B, collision_info *Info)
{
    std::vector<v2> Simplex;
    
    v2 Direction = Normalize(B->P - A->P);
    
    v2 P0 = FurthestPointInShape(A->P, &A->Shape, Direction) - FurthestPointInShape(B->P, &B->Shape, -Direction);
    Simplex.push_back(P0);
    
    Direction = -Direction;
    
    while(1)
    {
        v2 P1 = FurthestPointInShape(A->P, &A->Shape, Direction) - FurthestPointInShape(B->P, &B->Shape, -Direction);
        Simplex.push_back(P1);
        
        if (Dot(Simplex.back(), Direction) <= 0.0f)
        {
            // The last v2 added was not passed the origin
            return false;
        }
        else
        {
            if (ContainsOrigin(Simplex, &Direction))
            {
                if (Info)
                {
                    *Info = GetCollisionInfo(Simplex, A->P, &A->Shape, B->P, &B->Shape);
                }
                
                return true;
            }
        }
    }
}


void DetectAndResolveCollisions(game_state *State, f32 dt)
{
    for (u32 IndexA = 0; IndexA < 5; ++IndexA)
    {
        for (u32 IndexB = IndexA + 1; IndexB < 5; ++IndexB)
        {
            assert(IndexA != IndexB);
            
            body *BodyA = &State->Bodies[IndexA];
            body *BodyB = &State->Bodies[IndexB];
            
            collision_info Collision;
            if (Intersects(BodyA, BodyB, &Collision))
            {
                f32 TotalInverseMass = BodyA->InverseMass + BodyB->InverseMass;
                if (!AlmostEqualRelative(TotalInverseMass, 0.0f))
                {
                    // 
                    // Interpenetration
                    f32 AShare = BodyA->InverseMass / TotalInverseMass;
                    
                    f32 dP = Collision.Depth + 0.001f; // Nudge it a bit further so that it is not colliding anymore
                    f32 dPa = AShare * dP;
                    f32 dPb = dP - dPa;
                    
                    v2 dPMaska = BodyA->dPMask;
                    v2 dPMaskb = BodyB->dPMask;
                    
                    BodyA->P += Hadamard(dPMaska, dPa * -Collision.N);
                    BodyB->P += Hadamard(dPMaskb, dPb *  Collision.N);
                    
                    
                    //
                    // Resolve collision (by changing the velocity, a.k.a. dPt)
                    v2 N = Collision.N;
                    v2 Va = BodyA->dP;
                    v2 Vb = BodyB->dP;
                    
                    BodyA->dP = Hadamard(dPMaska, -2.0f * Dot(N, Va)*N + Va);BodyB->dP = Hadamard(dPMaskb, -2.0f * Dot(N, Vb)*N + Vb);
                }
            }
        }
    }
}


void UpdateBody(body& Body, f32 dt)
{
    assert(dt > 0.0f);
    
    Body.PrevP = Body.P;
    Body.P += Body.dP * dt;
    
    v2 ddP = Body.F * Body.InverseMass; // F = ma . a = F/m . a = F * (1/m)
    Body.F = v2_zero;
    
    Body.dP += Hadamard(Body.dPMask, ddP * dt - Body.Damping * Body.dP);
    
    if (Body.dP.y > Body.dPMax.y)
    {
        printf("Full speta!\n");
    }
    
    //
    // Speed limit
    Body.dP = V2(Min(Body.dP.x, Body.dPMax.x),
                 Min(Body.dP.y, Body.dPMax.y));
}


void UpdateAllBodies(game_state *State, f32 dt)
{
    assert(State);
    assert(dt > 0.0f);
    
    for (auto& Body : State->Bodies)
    {
        UpdateBody(Body, dt);
    }
    
    DetectAndResolveCollisions(State, dt);
}