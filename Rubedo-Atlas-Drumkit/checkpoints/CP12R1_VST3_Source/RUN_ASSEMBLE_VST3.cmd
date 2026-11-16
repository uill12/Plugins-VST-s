@echo off
setlocal
cd /d "%~dp0"
set "CORE=%~dp0Rubedo_CP12R1_VST3_CORE_ONLY.vst3"
set "CLAP=C:\Program Files\Common Files\CLAP\Rubedo_Atlas_Drumkit.clap"
if not exist "%CORE%" (
  echo Missing VST3 Windows core: %CORE%
  echo Download the verified SourceOnly GitHub Actions artifact and extract it here.
  pause
  exit /b 2
)
if not exist "%CLAP%" (
  echo Cannot find installed CP12R1 CLAP: %CLAP%
  echo Copy the exact CP12R1 Rubedo_Atlas_Drumkit.clap next to this script and change the CLAP path above.
  pause
  exit /b 3
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0AssembleRubedoCP12R1VST3.ps1" -ClapPath "%CLAP%" -VST3CorePath "%CORE%" -OutputRoot "%~dp0Rubedo_CP12R1_VST3_Assembly"
if errorlevel 1 (echo Assembly failed. Preserve your existing CLAP and previous VST3.& pause & exit /b 1)
echo Assembly complete. This build is not yet Bitwig-certified. Install only after backup.
pause
