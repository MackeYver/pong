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



static void SetupAsPaddle(dynamics_state *Dynamics, entity *Entity, mesh_index MIndex, texture_index TIndex, v2 Size, v2 VelMax)
{
    Init(Entity);
    Entity->Type = EntityType_Paddle;
    
    f32 Area = Size.x * Size.y;
    f32 Density = 0.0005f;
    f32 InverseMass = 1.0f / (Area * Density);
    
    body *Body = NewRectangleBody(Dynamics, Size, Entity);
    Body->dPMax = VelMax;
    Body->dPMask = V2(0.0f, 1.0f);
    Body->Damping = 0.5f;
    Body->InverseMass = InverseMass;
    
    Entity->BodyIndex = GetBodyIndex(Dynamics, Body);
    Entity->Size = Size;
    Entity->Scale = Size;
    Entity->MeshIndex = MIndex;
    Entity->TextureIndex = TIndex;
}


static void SetupPaddles(resources *Resources, dynamics_state *Dynamics, entity_pool *Pool, 
                         mesh_index MeshIndex, entity **EntityPtrs)
{
    texture_index TextureIndex = LoadBMP(Resources, "data\\bitmaps\\paddle.bmp");
    assert(TextureIndex >= 0);
    
    v2 VelocityMax = V2(3000.0f, 3000.0f);
    v2 Size = V2(25.0f, 140.0f);
    
    for (int Index = 0; Index < 2; ++Index)
    {
        entity *Entity = NewEntity(Pool);
        EntityPtrs[Index] = Entity;
        SetupAsPaddle(Dynamics, Entity, MeshIndex, TextureIndex, Size, VelocityMax);
    }
}
