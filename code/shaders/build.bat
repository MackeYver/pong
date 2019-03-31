@echo off
SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION


IF NOT "%1"=="nc" (
cls
)


IF NOT EXIST ..\..\build\shaders mkdir ..\..\build\shaders
PUSHD ..\..\build\shaders

IF EXIST vbasic.cso DEL /Q vbasic.cso
IF EXIST pbasic.cso DEL /Q pbasic.cso

IF EXIST vfullscreen_texture.cso DEL /Q vfullscreen_texture.cso
IF EXIST pfullscreen_texture.cso DEL /Q pfullscreen_texture.cso



SET Options=/nologo /O2 /WX /Zpr

REM Basic Shader
REM ------------
FXC !Options! /T vs_5_0 /E "vMain" /Fo vbasic.cso ..\..\code\shaders\basic.hlsl
IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

FXC !Options! /T ps_5_0 /E "pMain" /Fo pbasic.cso ..\..\code\shaders\basic.hlsl
IF !errorlevel! NEQ 0 EXIT /b !errorlevel!



REM Final rendering
REM ---------------
FXC !Options! /T vs_5_0 /E "vMain" /Fo vfullscreen_texture.cso ..\..\code\shaders\fullscreen_texture.hlsl
IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

FXC !Options! /T ps_5_0 /E "pMain" /Fo pfullscreen_texture.cso ..\..\code\shaders\fullscreen_texture.hlsl
IF !errorlevel! NEQ 0 EXIT /b !errorlevel!


POPD