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

#include "entity_pool.h"
#include "entity.h"




//
// Linked list
//

struct free_node
{
    entity *Ptr = nullptr;
    free_node *Next = nullptr;
};


entity *GetFreeLocation(entity_pool *Pool)
{
    entity *Result = nullptr;
    
    free_node *Node = Pool->FreeList;
    
    if (Node)
    {
        Result = Node->Ptr;
        Pool->FreeList = Node->Next;
        free(Node);
    }
    else
    {
        size_t Size = sizeof(entity);
        if (RemainingSize(&Pool->Memory) < Size)
        {
            printf("%s: out of memory, will try to resize the memory arena.\n", __FILE__);
            b32 bResult = Resize(&Pool->Memory, 2 * Pool->Memory.Size);
            
            if (!bResult)
            {
                printf("%s: Failed to resize memory arena!\n", __FILE__);
                return nullptr;
            }
        }
        Result = reinterpret_cast<entity *>(Push(&Pool->Memory, Size));
    }
    
    return Result;
}


void AddFreeLocation(entity_pool *Pool, entity *Entity)
{
    free_node *Node = Pool->FreeList;
    
    if (Node)
    {
        while (Node->Next)
        {
            Node = Node->Next;
        }
        
        Node->Next = static_cast<free_node *>(calloc(1, sizeof(free_node)));
        Node = Node->Next;
    }
    else
    {
        Pool->FreeList = static_cast<free_node *>(calloc(1, sizeof(free_node)));
        Node = Pool->FreeList;
    }
    
    Node->Ptr = Entity;
}




//
// Init and shutdown
//

void Init(entity_pool *Pool)
{
    size_t Size = 10 * sizeof(entity); // @debug
    Init(&Pool->Memory, Size);
}


void Shutdown(entity_pool *Pool)
{
    u32 Capacity = Pool->Memory.Size / sizeof(entity);
    for (u32 Index = 0; Index < Capacity; ++Index)
    {
        entity *Entity = &reinterpret_cast<entity *>(Pool->Memory.Ptr)[Index];
        
        if (Entity->Type == EntityType_Null)
        {
            continue;
        }
        
        Shutdown(Entity);
    }
    
    Free(&Pool->Memory);
}




//
// Create and remove entity
//

entity *NewEntity(entity_pool *Pool)
{
    entity *Result = GetFreeLocation(Pool);
    
    ++Pool->EntityCount;
    
    return Result;
}


void RemoveEntity(entity_pool *Pool, entity *Entity)
{
    AddFreeLocation(Pool, Entity);
    
#ifdef DEBUG
    memset(Entity, 0, sizeof(entity));
#endif
    
    --Pool->EntityCount;
}


void UpdateAll(dynamics_state *Dynamics, entity_pool *Pool, f32 dt)
{
    u32 Capacity = Pool->Memory.Size / sizeof(entity);
    for (u32 Index = 0; Index < Capacity; ++Index)
    {
        entity *Entity = &reinterpret_cast<entity *>(Pool->Memory.Ptr)[Index];
        
        if (Entity->Type == EntityType_Null)
        {
            continue;
        }
        
        Update(Dynamics, Entity, dt);
    }
}


void RenderAll(draw_calls *DrawCalls, entity_pool *Pool, b32 RenderAsPrimitives)
{
    u32 Capacity = Pool->Memory.Size / sizeof(entity);
    for (u32 Index = 0; Index < Capacity; ++Index)
    {
        entity *Entity = &reinterpret_cast<entity *>(Pool->Memory.Ptr)[Index];
        
        if (Entity->Type == EntityType_Null)
        {
            continue;
        }
        
        if (RenderAsPrimitives)
        {
            RenderBody(DrawCalls, Entity);
        }
        else
        {
            Render(DrawCalls, Entity);
        }
    }
}