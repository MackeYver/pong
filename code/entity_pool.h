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

#ifndef entity_pool__h
#define entity_pool__h

#include "memory_arena.h"
#include "mathematics.h"

struct entity;
struct draw_calls;
struct dynamics_state;
struct free_node;

struct entity_pool
{
    memory_arena Memory;
    u32 EntityCount = 0;
    
    free_node *FreeList = nullptr;
};


void Init(entity_pool *Pool);
void Shutdown(entity_pool *Pool);

entity *NewEntity(entity_pool *Pool);
void RemoveEntity(entity_pool *Pool, entity *Entity);

void UpdateAll(dynamics_state *Dynamics, entity_pool *Pool, f32 dt);
void RenderAll(draw_calls *DrawCalls, entity_pool *Pool, b32 RenderAsPrimitives);


#endif