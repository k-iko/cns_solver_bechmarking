@echo off
setlocal enabledelayedexpansion

cd .\code\pybind11
if not exist "build" (
    mkdir "build"
)
cd .\build
if exist "CMakeCache.txt" (
    del CMakeCache.txt
)

echo ===============
echo Generators List
echo ===============
cmake -G 2> generator.txt
set cnt=0
for /f "tokens=1 delims==" %%a in (generator.txt) do (
    set name=%%a
    echo !name! | find "Visual Studio" > NUL
    if not errorlevel 1 (
        set /a cnt=cnt+1
        echo !cnt! !name!
    )
)
echo ===============
set /p num="select generator (1~%cnt%): "
if %num% gtr %cnt% (
    del generator.txt
    echo too large
    pause
    exit \b 0
)
if %num% lss 1 (
    del generator.txt
    echo too small
    pause
    exit \b 0
)

set generator=""
set cnt=0
for /f "tokens=1 delims==" %%a in (generator.txt) do (
    set name=%%a
    echo !name! | find "Visual Studio" > NUL
    if not errorlevel 1 (
        set /a cnt=cnt+1
        if !cnt!==%num% (
            set generator=!name!
        )
    )
)
del generator.txt
set generator=%generator:~2%
echo %generator% | find "arch" > NUL
if not errorlevel 1 (
    if "%PROCESSOR_ARCHITECTURE%" NEQ "x86" (
        set generator=%generator:[arch]=Win64%
    ) else (
        set generator=%generator:[arch]=ARM%
    )
)
:loop
if "%generator:~-1%"==" " (
    set generator=%generator:~0,-1%
    goto loop
)
echo %generator%

cmake -G "%generator%" ..
if errorlevel 1 (
    echo ===============
    echo Error Occurred
    pause
    exit \b 0
)
cmake --build . --config Release
if errorlevel 1 (
    echo ===============
    echo Error Occurred
    pause
    exit \b 0
)
copy .\Release\*.pyd ..\..\

echo ===============
echo Done.

pause
exit \b 0