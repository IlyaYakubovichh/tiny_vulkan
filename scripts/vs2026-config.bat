@echo off
echo Generating Visual Studio 2026 project...

cd ../build

cmake .. ^
    -G "Visual Studio 18 2026" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=Debug

pause
