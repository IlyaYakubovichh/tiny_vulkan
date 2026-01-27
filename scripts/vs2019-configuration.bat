@echo off
echo Generating Visual Studio 2019 project...

cd ../build

REM Генерируем VS2019 x64 проект
cmake .. ^
    -G "Visual Studio 16 2019" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=Debug

pause