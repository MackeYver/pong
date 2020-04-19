@echo off
SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION


IF NOT "%1"=="nc" (
cls
)


IF NOT EXIST ..\..\run_tree\data\shaders mkdir ..\..\run_tree\data\shaders
PUSHD ..\..\run_tree\data\shaders

IF EXIST vprimitive.cso DEL /Q vprimitive.cso
IF EXIST pprimitive.cso DEL /Q pprimitive.cso

IF EXIST vtextured.cso DEL /Q vtextured.cso
IF EXIST ptextured.cso DEL /Q ptextured.cso

IF EXIST vfullscreen_texture.cso DEL /Q vfullscreen_texture.cso
IF EXIST pfullscreen_texture.cso DEL /Q pfullscreen_texture.cso


SET Options=/nologo /O2 /WX /Zpr


REM Primitives Shader
REM -----------------
FXC !Options! /T vs_5_0 /E "vMain" /Fo primitives_vs.cso ..\..\..\code\shaders\primitives.hlsl
IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

FXC !Options! /T ps_5_0 /E "pMain" /Fo primitives_ps.cso ..\..\..\code\shaders\primitives.hlsl
IF !errorlevel! NEQ 0 EXIT /b !errorlevel!



REM Basic textured Shader
REM ---------------------
FXC !Options! /T vs_5_0 /E "vMain" /Fo textured_vs.cso ..\..\..\code\shaders\textured.hlsl
IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

FXC !Options! /T ps_5_0 /E "pMain" /Fo textured_ps.cso ..\..\..\code\shaders\textured.hlsl
IF !errorlevel! NEQ 0 EXIT /b !errorlevel!



REM Final rendering
REM ---------------
FXC !Options! /T vs_5_0 /E "vMain" /Fo fullscreen_texture_vs.cso ..\..\..\code\shaders\fullscreen_texture.hlsl
IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

FXC !Options! /T ps_5_0 /E "pMain" /Fo fullscreen_texture_ps.cso ..\..\..\code\shaders\fullscreen_texture.hlsl
IF !errorlevel! NEQ 0 EXIT /b !errorlevel!


POPD
