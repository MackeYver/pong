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

#include "entity.h"
#include "draw_calls.h"

#include "entity_ball.h"
#include "entity_wall.h"
#include "entity_paddle.h"

#ifdef DEBUG
#include <stdio.h>
#else
#define printf(...)
#endif



//
// Init and shutdwon
//

void Init(entity *Entity)
{
    Entity->Colour = v4_one;
    Entity->P = v3_zero; 
    Entity->Size = v2_one;
    Entity->Scale = v2_one;
    Entity->BodyIndex = -1;
    Entity->MeshIndex = -1;
    Entity->TextureIndex = -1;
}

void Shutdown(entity *Entity)
{
}




//
// Update
//

void Update(dynamics_state *Dynamics, entity *Entity, f32 dt)
{
    body *Body = GetBody(Dynamics, Entity->BodyIndex);
    Entity->P = V3(Body->P, 0.0f);
}




//
// Render
//

void Render(draw_calls *DrawCalls, entity *Entity, b32 UseRetroMode)
{
    switch (Entity->Type)
    {
        case EntityType_Ball:
        {
            RenderAsBall(DrawCalls, Entity, UseRetroMode);
        } break;
        
        case EntityType_Paddle:
        {
            RenderAsPaddle(DrawCalls, Entity, UseRetroMode);
        } break;
        
        case EntityType_Wall:
        {
            RenderAsWall(DrawCalls, Entity, UseRetroMode);
        } break;
        
        default:
        {
            printf("%s: Tried to render unknown entity type!\n", __FILE__);
        } break;
    }
}
