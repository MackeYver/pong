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

#include "mesh.h"
#include "tokenizer.h"
#include "mathematics.h"

#include <stdlib.h>
#include <stdio.h>


#ifdef DEBUG
#include <assert.h>
#define Assertu16(x) assert(x < 65536)
#else
#define assert(x)
#define Assertu16(x)
#endif



//
// Ply
enum ply_token
{
    PlyToken_MagicNumber,
    PlyToken_Format,
    PlyToken_Comment,
    PlyToken_Element,
    PlyToken_Property,
    PlyToken_EndHeader,
    PlyToken_Unknown,
};


ply_token TranslateToPlyToken(token *Token)
{
    if (StringsAreEqual(Token->Text, "ply"))
    {
        return PlyToken_MagicNumber;
    }
    else if (StringsAreEqual(Token->Text, "format"))
    {
        return PlyToken_Format;
    }
    else if (StringsAreEqual(Token->Text, "comment"))
    {
        return PlyToken_Comment;
    }
    else if (StringsAreEqual(Token->Text, "element"))
    {
        return PlyToken_Element;
    }
    else if (StringsAreEqual(Token->Text, "property"))
    {
        return PlyToken_Property;
    }
    else if (StringsAreEqual(Token->Text, "end_header"))
    {
        return PlyToken_EndHeader;
    }
    else
    {
        return PlyToken_Unknown;
    }
}


token ProcessFormat(tokenizer *Tokenizer, mesh *State)
{
    token Token = RequireIdentifierNamed(Tokenizer, "ascii"); // ascii
    Token = RequireNumberWithValue(Tokenizer, 1.0f); // 1.0
    
    Token = GetToken(Tokenizer);
    return Token;
}


token RequireFloatPropertyNamed(tokenizer *Tokenizer, char const *Name)
{
    token Token = RequireIdentifierNamed(Tokenizer, "property");
    Token = RequireIdentifierNamed(Tokenizer, "float");
    Token = RequireIdentifierNamed(Tokenizer, Name);
    
    return Token;
}


token ProcessElementInHeader(tokenizer *Tokenizer, b32 *HasVertices, b32 *HasNormals, b32 *HasUVs, mesh *Output)
{
    token Token = GetToken(Tokenizer);
    
    *HasVertices = false;
    *HasNormals = false;
    *HasUVs = false;
    
    if (StringsAreEqual(Token.Text, "vertex"))
    {
        Token = RequireNumber(Tokenizer);
        Output->VertexCount = Token.s32;
        
        // NOTE(Marcus): We're requiring that the vertices has at least three properties:
        //               x, y and z (floats), the position
        //
        //               In addition to this they _can_ have (they are optional) additional properties:
        //               - nx, ny and nz (floats), this is the normal of the vertex
        //               - s and t, this is the texture coordinate
        
        Token = RequireFloatPropertyNamed(Tokenizer, "x");
        Token = RequireFloatPropertyNamed(Tokenizer, "y");
        Token = RequireFloatPropertyNamed(Tokenizer, "z");
        
        Output->VertexSize = sizeof(v3);
        *HasVertices = true;
        
        //
        // Check if there are any other properties of the vertex
        Token = GetToken(Tokenizer);
        ply_token PlyToken = TranslateToPlyToken(&Token);
        while (Parsing(Tokenizer) && (PlyToken == PlyToken_Property))
        {
            Token = RequireIdentifierNamed(Tokenizer, "float");
            Token = GetToken(Tokenizer);
            
            if (StringsAreEqual(Token.Text, "nx"))
            {
                Token = RequireFloatPropertyNamed(Tokenizer, "ny");
                Token = RequireFloatPropertyNamed(Tokenizer, "nz");
                
                Output->VertexSize += sizeof(v3);
                *HasNormals = true;
            }
            else if (StringsAreEqual(Token.Text, "s"))
            {
                Token = RequireFloatPropertyNamed(Tokenizer, "t");
                
                Output->VertexSize += sizeof(v2);
                *HasUVs = true;
            }
            Token = GetToken(Tokenizer);
            PlyToken = TranslateToPlyToken(&Token);
        }
    }
    else if (StringsAreEqual(Token.Text, "face"))
    {
        // NOTE(Marcus): We're requring the face to consist out of three indices, i.e. we're
        //               requiring the faces to be triangulated.
        Token = RequireNumber(Tokenizer);
        Output->FaceCount = Token.s32;
        Output->IndexCount = 3*Output->FaceCount;
        Token = GetFirstTokenOfNextLine(Tokenizer);
    }
    else
    {
        // TODO(Marcus): Support arbitrary named elements? For now we'll stop parsing here because
        //               we did not expect this unknown element -- perhaps there were errors in 
        //               the export?
        Error(Tokenizer, "%d: Unsupported element", Token.LineNumber);
    }
    
    return Token;
}


token EndHeader(tokenizer *Tokenizer, b32 HasVertices, b32 HasNormals, b32 HasUVs, mesh *Output)
{
    assert(HasVertices);
    
    if (Output->VertexCount == 0)
    {
        Error(Tokenizer, "Finished parsing the header but found no vertices, or no vertices which also has positions");
    }
    else
    {
        Output->Positions = (v3 *)malloc(Output->VertexCount * sizeof(v3));
        
        if (HasNormals)
        {
            Output->Normals = (v3 *)malloc(Output->VertexCount * sizeof(v3));
        }
        
        if (HasUVs)
        {
            Output->UVs = (v2 *)malloc(Output->VertexCount * sizeof(v2));
        }
        
        if (Output->IndexCount > 0)
        {
            Output->Indices = (u16 *)malloc(Output->IndexCount * sizeof(u16));
        }
    }
    
    token Token = GetToken(Tokenizer);
    return Token;
}



//
// "Public" functions
b32 ParsePLY(u8 *Data, size_t DataSize, mesh *Output)
{
    b32 Result = false;
    
    b32 FinishedWithHeader = false;
    b32 HasVertices = false;
    b32 HasNormals = false;
    b32 HasUVs = false;
    
    if (Data && DataSize > 0 && Output)
    {
        tokenizer Tokenizer;
        
        string FileString;
        FileString.Data = Data;
        FileString.Count = DataSize;
        snprintf(Tokenizer.FileName, TokenizerMaxStringLength, "test"); // TODO(Marcus): We're not using the filename here
        
        
        //
        // Tokenize and parse
        token Token;
        Token = RequireIdentifierNamed(&Tokenizer, "ply");
        if (!Parsing(&Tokenizer))
        {
            Error(&Tokenizer, "First line did not contain the magic number, is this really a .ply file?");
        }
        Token = GetToken(&Tokenizer);
        
        while(Parsing(&Tokenizer))
        {
            //Token = GetToken(&Tokenizer);
            // NOTE(Marcus): Each process function in the switch case below will advance the tokenizer
            //               and return the next token (i.e. the token that comes after the last token
            //               the function processed).
            
            switch (Token.Type)
            {
                case Token_Identifier:
                {
                    ply_token PlyToken = TranslateToPlyToken(&Token); 
                    switch (PlyToken)
                    {
                        case PlyToken_Element: { 
                            Token = ProcessElementInHeader(&Tokenizer, &HasVertices, &HasNormals, &HasUVs, Output);
                        } break;
                        
                        case PlyToken_EndHeader: { 
                            Token = EndHeader(&Tokenizer, HasVertices, HasNormals, HasUVs, Output);
                        } break;
                        
                        case PlyToken_Format   : { Token = ProcessFormat(&Tokenizer, Output);      } break;
                        case PlyToken_Comment  : { Token = GetFirstTokenOfNextLine(&Tokenizer);    } break;
                        case PlyToken_Property : { Token = GetFirstTokenOfNextLine(&Tokenizer);    } break;
                        case PlyToken_Unknown  : { Error(&Tokenizer, Token, "Unknown identifier"); } break;
                    }
                } break;
                
                case Token_Number:
                {
                    if (!FinishedWithHeader)
                    {
                        Error(&Tokenizer, Token, "Encountered free-form numbers before the header was parsed");
                    }
                    else
                    {
                        // NOTE(Marcus): We're assuming that the first time we encounter a number, _and_
                        //               the header was parsed, then it is the start of the vertex data.
                        for (u32 Index = 0; Index < Output->VertexCount && Parsing(&Tokenizer); ++Index)
                        {
                            f32 x = Token.f32;
                            Token = RequireNumber(&Tokenizer);
                            f32 y = Token.f32;
                            Token = RequireNumber(&Tokenizer);
                            f32 z = Token.f32;
                            Output->Positions[Index] = V3(x, y, z);
                            
                            Token = GetToken(&Tokenizer);
                            if (HasNormals && Token.Type == Token_Number)
                            {
                                f32 nx = Token.f32;
                                Token = RequireNumber(&Tokenizer);
                                f32 ny = Token.f32;
                                Token = RequireNumber(&Tokenizer);
                                f32 nz = Token.f32;
                                Output->Normals[Index] = V3(nx, ny, nz);
                            }
                            
                            Token = GetToken(&Tokenizer);
                            if (HasUVs && Token.Type == Token_Number)
                            {
                                f32 s = Token.f32;
                                Token = RequireNumber(&Tokenizer);
                                f32 t = Token.f32;
                                Output->UVs[Index] = V2(s, t);
                                
                                Token = GetToken(&Tokenizer);
                            }
                            
                        }
                        
                        for (u32 Index = 0; Index < Output->FaceCount && Parsing(&Tokenizer); ++Index)
                        {
                            if (Token.s32 != 3)
                            {
                                Error(&Tokenizer, Token, "Expected 3 indices per face but got %d", Token.s32);
                            }
                            else
                            {
                                Token = RequireNumber(&Tokenizer);
                                Assertu16(Token.s32);
                                Output->Indices[3*Index] = (u16)Token.s32;
                                
                                Token = RequireNumber(&Tokenizer);
                                Assertu16(Token.s32);
                                Output->Indices[3*Index + 1] = (u16)Token.s32;
                                
                                Token = RequireNumber(&Tokenizer);
                                Assertu16(Token.s32);
                                Output->Indices[3*Index + 2] = (u16)Token.s32;
                                
                                if (Index < Output->FaceCount - 1)
                                {
                                    Token = RequireNumber(&Tokenizer);
                                } else
                                {
                                    Token = GetToken(&Tokenizer);
                                }
                            }
                        }
                    }
                } break;
            }
        }
        
        if (Tokenizer.Error)
        {
            Free(Output);
        }
        else
        {
            Result = true;
        }
    }
    
    return Result;
}


void Free(mesh *Mesh)
{
    if (Mesh->Positions)
    {
        free(Mesh->Positions);
        Mesh->Positions = nullptr;
    }
    
    if (Mesh->Normals)
    {
        free(Mesh->Normals);
        Mesh->Normals = nullptr;
    }
    
    if (Mesh->UVs)
    {
        free(Mesh->UVs);
        Mesh->UVs = nullptr;
    }
    
    if (Mesh->Indices)
    {
        free(Mesh->Indices);
        Mesh->Indices = nullptr;
    }
    
    Mesh->VertexCount = 0;
    Mesh->VertexSize = 0;
    Mesh->IndexCount = 0;
}