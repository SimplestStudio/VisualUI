@echo off

setlocal
cd /d "%~dp0.."

for %%c in (Debug Release) do (
    echo Removing old build directory for %%c...
    rd /s /q "lib\build\%%c" 2>nul

    echo Configuring %%c for x86...
    cmake -S . -B "lib/build/%%c" -G "Visual Studio 16 2019" -A Win32
    if errorlevel 1 exit /b 1

    echo Building %%c...
    cmake --build "lib/build/%%c" --config %%c
    if errorlevel 1 exit /b 1

    echo Installing %%c...
    cmake --install "lib/build/%%c" --config %%c
    if errorlevel 1 exit /b 1
)

echo Build and install completed successfully.
