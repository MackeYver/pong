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

#include "resources.h"
#include "dynamics.h"
#include "entity_pool.h"



//
// Init
// 

static mesh_index CreateWallMesh(resources *Resources, v2 Size)
{
    //
    // Sizes and stuff
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
    
    mesh_index MeshIndex = LoadMesh(Resources, P, UV, 6);
    
    return MeshIndex;
}


static void SetupAsWall(dynamics_state *Dynamics, entity *Entity, mesh_index MeshIndex, texture_index TextureIndex, v2 P, v2 Size)
{
    Init(Entity);
    Entity->Type = EntityType_Wall;
    
    body *Body = NewRectangleBody(Dynamics, Size, Entity);
    Body->P = P;
    Body->InverseMass = 0.0f;
    Body->dPMask = v2_zero;
    
    Entity->MeshIndex = MeshIndex;
    Entity->TextureIndex = TextureIndex;
    Entity->BodyIndex = GetBodyIndex(Dynamics, Body);
    Entity->Size = Size;
}


static void InitWalls(resources *Resources, dynamics_state *Dynamics, entity_pool *Pool, v2 WindowSize, entity **EntityPtr)
{
    v2  const Size = Hadamard(V2(1.0f, 0.05f), WindowSize);
    f32 const MidX = 0.5f * WindowSize.x;
    f32 const Y[2] = {(f32)WindowSize.y - 0.5f * Size.y, 0.5f * Size.y};
    
    mesh_index MeshIndex = CreateWallMesh(Resources, Size);
    assert(MeshIndex >= 0);
    
    texture_index TextureIndex = LoadBMP(Resources, "data\\brick.bmp");
    assert(TextureIndex >= 0);
    
    for (int Index = 0; Index < 2; ++Index)
    {
        entity *Entity = NewEntity(Pool);
        EntityPtr[Index] = Entity;
        SetupAsWall(Dynamics, Entity, MeshIndex, TextureIndex, V2(MidX, Y[Index]), Size);
    }
}




//
// Render
//

static void RenderAsWall(draw_calls *DrawCalls, entity *Entity, b32 Retro)
{
    if (Retro)
    {
        v2 HalfSize = 0.5f * Entity->Size;
        v3 P0 = Entity->P - V3(HalfSize.x, HalfSize.y, 0.0f);
        v3 P1 = Entity->P + V3(HalfSize.x, HalfSize.y, 0.0f);
        PushPrimitiveRectangleFilled(DrawCalls, P0, P1, Entity->Colour);
    }
    else
    {
        PushTexturedMesh(DrawCalls, Entity->P, Entity->MeshIndex, Entity->TextureIndex, Entity->Scale, Entity->Colour);
    }
}