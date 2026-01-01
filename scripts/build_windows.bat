@echo off

setlocal
cd /d "%~dp0.."

for %%c in (Debug Release) do (
    echo Configuring %%c...
    cmake -S . -B "lib/build/%%c"
    if errorlevel 1 exit /b 1

    echo Building %%c...
    cmake --build "lib/build/%%c" --config %%c
    if errorlevel 1 exit /b 1

    echo Installing %%c...
    cmake --install "lib/build/%%c" --config %%c
    if errorlevel 1 exit /b 1
)

echo Build and install completed successfully.
