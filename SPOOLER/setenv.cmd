@echo off
rem setenv.cmd - set up the build environment for the OS/2 Spooler Clone project.
rem Run this once before invoking wmake (from any sub-directory or the top level).
rem
rem Usage:
rem   [F:\...\PROJECT] setenv.cmd
rem   [F:\...\PROJECT] wmake

set TKBASE=F:\Temporal\1.-OS2\5.- AI Replacements\references\IBM_OS2Toolkit_4-5r0\os2tk45

set INCLUDE=%TKBASE%\h;%INCLUDE%
set LIB=%TKBASE%\lib;%LIB%

echo Build environment set.
echo   INCLUDE = %INCLUDE%
echo   LIB     = %LIB%
