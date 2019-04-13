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

#ifndef dynamics__h
#define dynamics__h

#include <vector> // @debug
#include "mathematics.h" // includes types.h


typedef s32 body_index;

enum shape_type
{
    ShapeType_Rectangle,
    ShapeType_Circle,
    
    ShapeType_Count,
};


struct shape
{
    s32 BodyIndex = -1;
    union
    {
        v2 HalfSize = V2(1.0f, 1.0f);
        f32 Radius;
    };
    shape_type Type = ShapeType_Rectangle;
};


struct body
{
    // The dynamics system will do nothing with this, neither free nor allocate any memory.
    void *UserData = nullptr; 
    
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


struct collision_info
{
    v2 N = v2_zero;
    body_index BodiesInvolved[2] = {};
    f32 ForceModifier = 0.0f;
    f32 Depth = 0.0f;
    b32 SkipInterpenetration = false;
    b32 SkipForceApplication = false;
};


struct dynamics_state
{
    std::vector<body> Bodies;
};

void Init(dynamics_state *State);
void Shutdown(dynamics_state *State);


//
// Add a new body to the simulation, returns the index of the body in the "body-array"
body_index NewRectangleBody(dynamics_state *State, v2 Size, void *UserData = nullptr);
body_index NewCircleBody(dynamics_state *State, f32 Radius, void *UserData = nullptr);
body *GetBody(dynamics_state *State, body_index BodyIndex);


//
// Update all the bodies
void Update(dynamics_state *State, f32 dt);

//
// Detect and resolve collisions
void DetectCollisions(dynamics_state *State, std::vector<collision_info>& Output);
void ResolveCollisions(dynamics_state *State, std::vector<collision_info>& Input, f32 dt);



#endif