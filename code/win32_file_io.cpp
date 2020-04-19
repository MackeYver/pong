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

#include "win32_file_io.h"

#define UNICODE
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>
#include <string> // @debug
#include <vector> // @debug

#ifdef DEBUG
#include <stdio.h>
#include <assert.h>
#else
#define printf(...)
#define assert(x)
#endif



//
// File reading using the win32 API.
//
b32 win32_ReadFile(char const *PathAndName, u8 **Data, u32 *DataSize)
{
    assert(PathAndName);
    assert(Data);
    assert(DataSize);
    
    HANDLE File = CreateFileA(PathAndName,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              nullptr,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr);
    if (File == INVALID_HANDLE_VALUE)
    {
#ifdef DEBUG
        DWORD Error = GetLastError();
        printf("Failed to open file, error: %d\n", Error);
#endif
        return false;
    }
    
    LARGE_INTEGER FileSize;
    GetFileSizeEx(File, &FileSize);
    assert(FileSize.QuadPart > 0);
    
    *Data = static_cast<u8 *>(malloc(FileSize.QuadPart));
    assert(Data);
    
    DWORD BytesRead = 0;
    if (!ReadFile(File, *Data, (DWORD)FileSize.QuadPart, &BytesRead, nullptr) || BytesRead != FileSize.QuadPart)
    {
        if (Data)  
        {
            free(Data);
            Data = nullptr;
        }
        
        CloseHandle(File);
        
#ifdef DEBUG
        DWORD Error = GetLastError();
        printf("Failed to read from file, error: %d", Error);
#endif
        
        return false;
    }
    
    *DataSize = static_cast<u32>(FileSize.QuadPart);
    
    CloseHandle(File);
    
    return true;
}
