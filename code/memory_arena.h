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

#ifndef memory_arena__h
#define memory_arena__h

#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif

#include "types.h"
#include <stdlib.h>



//
// Struct
//

struct memory_arena
{
    u8 *Ptr = nullptr;
    size_t Size = 0;
    size_t Used = 0;
};




//
// Implementation
//

static void Free(memory_arena *Memory)
{
    assert(Memory);
    if (Memory->Ptr)
    {
        free(Memory->Ptr);
        Memory->Ptr = nullptr;
    }
    
    Memory->Size = 0;
    Memory->Used = 0;
}


static b32 Init(memory_arena *Memory, size_t Size)
{
    Free(Memory);
    b32 Result = false;
    
    if (Memory && Size > 0)
    {
        Memory->Ptr = static_cast<u8 *>(calloc(1, Size));
        assert(Memory->Ptr);
        
        Memory->Size = Size;
        Memory->Used = 0;
    }
    
    return Result;
}


static size_t RemainingSize(memory_arena *Memory)
{
    size_t Result = Memory->Size - Memory->Used;
    
    return Result;
}


static b32 Resize(memory_arena *Memory, size_t NewSize)
{
    b32 Result = false;
    
    if (Memory && NewSize > 0)
    {
        u8 *NewPtr = static_cast<u8 *>(realloc(Memory->Ptr, NewSize));
        
        if (NewPtr)
        {
            Memory->Ptr = NewPtr;
            Memory->Size = NewSize;
            Result = true;
        }
    }
    
    return Result;
}


static u8 *Push(memory_arena *Memory, size_t Size)
{
    assert(Memory);
    
    u8 *Result = nullptr;
    
    if ((Memory->Size - Memory->Used) >= Size)
    {
        Result = (Memory->Ptr + Memory->Used);
        Memory->Used += Size;
    }
    
    return Result;
}


static void Clear(memory_arena *Memory)
{
    assert(Memory);
    Memory->Used = 0;
}



#endif
