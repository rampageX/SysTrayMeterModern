#!/bin/sh
set -eu
cd "$(dirname "$0")"
(cd src && x86_64-w64-mingw32-windres app.rc -O coff -o ../app-res.o)
x86_64-w64-mingw32-g++ -std=c++17 -Os -Wall -Wextra -Wpedantic -municode -mwindows -static -static-libgcc -static-libstdc++ -s -Wl,--nxcompat,--dynamicbase src/main.cpp app-res.o -lpdh -lshell32 -luser32 -lgdi32 -o SysTrayMeterModern.exe
rm app-res.o
