@echo off
title DecoPilot Local AI Setup
echo.
echo DecoPilot uses Ollama locally. No paid API key is required.
echo Recommended for an RTX 3070: qwen2.5vl:7b
echo.
where ollama >nul 2>nul
if errorlevel 1 (
  echo ERROR: Ollama was not found in PATH.
  echo Install Ollama first, then run this file again.
  pause
  exit /b 1
)

echo Pulling qwen2.5vl:7b...
ollama pull qwen2.5vl:7b
if errorlevel 1 (
  echo.
  echo The 7B model failed. You can try the lighter fallback:
  echo   ollama pull gemma3:4b
  pause
  exit /b 1
)

echo.
echo Done. Keep Ollama running, start Geometry Dash, open the editor,
echo press ESC, then click AI Deco.
pause
