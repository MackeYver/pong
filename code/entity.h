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

#ifndef entity__h
#define entity__h

#include "mathematics.h"
#include "resources.h"
#include "dynamics.h"
#include "draw_calls.h"


enum entity_type
{
    EntityType_Null = 0,
    EntityType_Ball,
    EntityType_Paddle,
    EntityType_Wall,
    
    EntityType_Count
};


struct entity
{
    entity_type Type = EntityType_Null;
    v4 Colour = v4_one;
    v3 P; // read-only! Note: this is probably a bad idea...
    v2 Size = v2_one;
    v2 Scale = v2_one;
    
    body_index BodyIndex = -1;
    mesh_index MeshIndex = -1;
    texture_index TextureIndex = -1;
};

void Init(entity *Entity);
void Shutdown(entity *Entity);

void Update(dynamics_state *Dynamics, entity *Entity, f32 dt);

void Render(draw_calls *DrawCalls, entity *Entity);
void RenderBody(draw_calls *DrawCalls, entity *Entity);

#include "entity_ball.h"
#include "entity_paddle.h"
#include "entity_wall.h"

#endif