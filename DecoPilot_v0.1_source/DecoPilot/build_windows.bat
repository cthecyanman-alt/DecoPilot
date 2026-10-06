@echo off
setlocal
cd /d "%~dp0"

echo === DecoPilot Windows build ===
where cmake >nul 2>nul || (
  echo ERROR: cmake was not found in PATH.
  pause
  exit /b 1
)

if "%GEODE_SDK%"=="" (
  echo ERROR: GEODE_SDK is not set.
  echo Install the Geode SDK and set GEODE_SDK to its folder.
  pause
  exit /b 1
)

cmake -B build -A x64
if errorlevel 1 goto :fail
cmake --build build --config Release
if errorlevel 1 goto :fail

echo.
echo Build complete. Look in the build folder for decopilot.local.geode.
pause
exit /b 0

:fail
echo.
echo Build failed. Copy the error output if you want help fixing it.
pause
exit /b 1
