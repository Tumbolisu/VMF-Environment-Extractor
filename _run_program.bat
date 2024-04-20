@echo off
echo [BAT] Running Program...
bin\main.exe %*  ||  goto :errorexe
echo [BAT] Finished without errors.
PAUSE
exit


:errorexe
echo [BAT] An error occured while running the program! Exit code: %ERRORLEVEL%
PAUSE
exit /b %ERRORLEVEL%
