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
#include "dynamics.h"
#include "resources.h"
#include "entity_pool.h"



static mesh_index CreateMeshFor1x1Quad(resources *Resources)
{
    //
    // Sizes and stuff
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
    
    mesh_index MeshIndex = LoadMesh(Resources, P, UV, 6);
    
    return MeshIndex;
}


static void SetupAsBall(resources *Resources, dynamics_state *Dynamics, entity_pool *Pool, mesh_index MeshIndex, entity **EntityPtr)
{
    v2 VelocityMax = V2(1500.0f, 1500.0f);
    
    entity *Entity = NewEntity(Pool);
    *EntityPtr = Entity;
    
    Init(Entity);
    Entity->Type = EntityType_Ball;
    
    v2 Size = V2(25.0f, 25.0f);
    f32 BallRadius = 12.5f;
    f32 BallArea = Pi32 * BallRadius * BallRadius;
    f32 BallDensity = 0.0004f;
    
    body *Body = NewCircleBody(Dynamics, BallRadius, Entity);
    Body->dPMax = VelocityMax;
    Body->InverseMass = 1.0f / (BallArea * BallDensity);
    
    Entity->Size = Size;
    Entity->Scale = Size;
    Entity->BodyIndex = GetBodyIndex(Dynamics, Body);
    Entity->MeshIndex = MeshIndex;
    
    texture_index TextureIndex = LoadBMP(Resources, "data\\ball.bmp");
    assert(TextureIndex >= 0);
    Entity->TextureIndex = TextureIndex;
}


static void RenderAsBall(draw_calls *DrawCalls, entity *Entity, b32 Retro)
{
    if (Retro)
    {
        PushPrimitiveCircleFilled(DrawCalls, Entity->P, 12.5f, Entity->Colour);
    }
    else
    {
        PushTexturedMesh(DrawCalls, Entity->P, Entity->MeshIndex, Entity->TextureIndex, Entity->Scale, Entity->Colour);
    }
}