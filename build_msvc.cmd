@echo off
setlocal
cd /d "%~dp0"
where cl >nul 2>nul
if errorlevel 1 (
 echo Open an x64 Native Tools Command Prompt for Visual Studio first.
 exit /b 1
)
pushd src
rc /nologo /fo ..\app-res.res app.rc
if errorlevel 1 (popd & exit /b 1)
popd
cl /nologo /std:c++17 /O1 /MT /EHsc /utf-8 /W4 src\main.cpp app-res.res /Fe:SysTrayMeterModern.exe /link /SUBSYSTEM:WINDOWS /MANIFEST:NO pdh.lib shell32.lib user32.lib gdi32.lib
if errorlevel 1 exit /b 1
del /q main.obj app-res.res 2>nul
