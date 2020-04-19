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

#include "dynamics.h"

#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif



//
// Structs
//

struct edge
{
    v2 N;
    f32 Distance;
    u32 VertexIndex;
};




//
// Forward declarations of "private" functions
//

//
// Simulation
void UpdateBody(body& Body, f32 dt);

//
// Collisions
void DetectAndResolveCollisions(dynamics_state *State, f32 dt);
b32 Intersects(body *A, body *B, collision_info *Info);
collision_info GetCollisionInfo(std::vector<v2>& Simplex, v2 Poa, shape *A, v2 Pob, shape *B);

// 
// Geometry related
b32 ContainsOrigin(std::vector<v2>& Simplex, v2 *Direction);
edge FindEdgeClosestToOrigin(std::vector<v2>& Simplex);
v2 FurthestPointInShape(v2 Po, shape *Shape, v2 Direction);
v2 FurthestPointInCircle(v2 Po, shape *Circle, v2 Direction);
v2 FurthestPointInRectangle(v2 Po, shape *Rectangle, v2 Direction);




//
// "Public" functions (declared in the header file).
//

void Init(dynamics_state *State)
{
    State->Bodies.clear();
}


void Shutdown(dynamics_state *State)
{
    State->Bodies.clear();
}


void Update(dynamics_state *State, f32 dt)
{
    assert(State);
    assert(dt > 0.0f);
    
    for (auto& Body : State->Bodies)
    {
        UpdateBody(Body, dt);
    }
}


body *NewRectangleBody(dynamics_state *State, v2 Size, void *UserData)
{
    assert(State);
    
    u32 Index = static_cast<u32>(State->Bodies.size());
    State->Bodies.emplace_back(body());
    body *Result = &State->Bodies.back();
    
    Result->Shape.Type = ShapeType_Rectangle;
    Result->Shape.HalfSize = 0.5f*Size;
    Result->Shape.BodyIndex = Index;
    Result->UserData = UserData;
    
    return Result;
}


body *NewCircleBody(dynamics_state *State, f32 Radius, void *UserData)
{
    assert(State);
    
    u32 Index = static_cast<u32>(State->Bodies.size());
    State->Bodies.emplace_back(body());
    body *Result = &State->Bodies.back();
    
    Result->Shape.Type = ShapeType_Circle;
    Result->Shape.Radius = Radius;
    Result->Shape.BodyIndex = Index;
    Result->UserData = UserData;
    
    return Result;
}


body *GetBody(dynamics_state *State, body_index BodyIndex)
{
    return &State->Bodies[BodyIndex];
}


body_index GetBodyIndex(dynamics_state *State, body *Body)
{
    body_index Result = -1;
    
    if (State && Body)
    {
        if (Body->Shape.BodyIndex >= 0)
        {
            Result = Body->Shape.BodyIndex;
        }
        else
        {
            for (std::vector<body>::size_type Index = 0; Index < State->Bodies.size(); ++Index)
            {
                if (&State->Bodies[Index] == Body)
                {
                    Result = static_cast<s32>(Index);
                    Body->Shape.BodyIndex = Result;
                    break;
                }
            }
        }
    }
    
    return Result;
}


void SetP(dynamics_state *State, body_index BodyIndex, v2 Po)
{
    body *Body = GetBody(State, BodyIndex);
    Body->P = Po;
    Body->PrevP = Body->P;
    
    Body->dP = v2_zero;
    Body->F = v2_zero;
}


void DetectCollisions(dynamics_state *State, std::vector<collision_info>& Output)
{
    for (u32 IndexA = 0; IndexA < 5; ++IndexA)
    {
        for (u32 IndexB = IndexA + 1; IndexB < 5; ++IndexB)
        {
            body *Body[2];
            Body[0] = &State->Bodies[IndexA];
            Body[1] = &State->Bodies[IndexB];
            
            collision_info Collision;
            if (Intersects(Body[0], Body[1], &Collision))
            {
                Collision.BodiesInvolved[0] = IndexA;
                Collision.BodiesInvolved[1] = IndexB;
                Collision.UserData[0] = Body[0]->UserData;
                Collision.UserData[1] = Body[1]->UserData;
                Output.push_back(Collision);
            }
        }
    }
}


void ResolveCollisions(dynamics_state *State, std::vector<collision_info>& Input, f32 dt)
{
    // TODO(Marcus): Check the math, we're currently not caring about dt, but we probably
    //               want to use impulse in our calculations?
    
    for (auto& Collision : Input)
    {
        body *Body[2];
        Body[0] = GetBody(State, Collision.BodiesInvolved[0]);
        Body[1] = GetBody(State, Collision.BodiesInvolved[1]);
        
        v2 dPMaska = Body[0]->dPMask;
        v2 dPMaskb = Body[1]->dPMask;
        
        f32 TotalInverseMass = Body[0]->InverseMass + Body[1]->InverseMass;
        
        
        // 
        // Interpenetration
        if (!Collision.SkipInterpenetration)
        {
            f32 ShareA;
            if (AlmostEqualRelative(TotalInverseMass, 0.0f))
            {
                ShareA = 0.5f;
            }
            else
            {
                ShareA = Body[0]->InverseMass / TotalInverseMass;
            }
            
            f32 dP = Collision.Depth + 0.005f; // Nudge it a bit further so that it is not colliding anymore
            f32 dPa = ShareA * dP;
            f32 dPb = dP - dPa;
            
            Body[0]->P -= Hadamard(dPMaska, dPa * Collision.N);
            Body[1]->P += Hadamard(dPMaskb, dPb * Collision.N);
        }
        
        
        //
        // Calculate forces due to the collision
        if (!Collision.SkipForceApplication)
        {
            v2 N = Collision.N;
            v2 Va = Body[0]->dP;
            v2 Vb = Body[1]->dP;
            
            f32 ForceCoeff = 1 + Collision.ForceModifier;
            Body[0]->dP = ForceCoeff * Hadamard(dPMaska, -2.0f * Dot(N, Va)*N + Va);
            Body[1]->dP = ForceCoeff * Hadamard(dPMaskb, -2.0f * Dot(N, Vb)*N + Vb);
        }
    }
}




//
// Simulation
//

void UpdateBody(body& Body, f32 dt)
{
    assert(dt > 0.0f);
    
    v2 ddP = Body.F * Body.InverseMass; // F = ma ->  a = F/m -> a = F * (1/m)
    Body.F = v2_zero;
    
    Body.dP += Hadamard(Body.dPMask, (ddP * dt) - (Body.Damping * Body.dP));
    
    Body.PrevP = Body.P;
    Body.P += Body.dP * dt;
    
    //
    // Speed limit
    Body.dP = V2(Min(Body.dP.x, Body.dPMax.x),
                 Min(Body.dP.y, Body.dPMax.y));
}




//
// Collision
//

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




//
// Geometry related
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


// Direction should be of unit length
v2 FurthestPointInShape(v2 Po, shape *Shape, v2 Direction)
{
    assert(Shape);
    
    v2 Result;
    
    switch (Shape->Type)
    {
        case ShapeType_Rectangle: 
        {
            Result = FurthestPointInRectangle(Po, Shape, Direction);
        } break;
        
        case ShapeType_Circle:
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
