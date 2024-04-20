@echo off
echo [BAT] Building Program...
make  ||  goto :errormake
echo [BAT] Finished without errors.
PAUSE
exit

:errormake
echo [BAT] An error occured while building the project! Exit code: %ERRORLEVEL%
PAUSE
exit /b %ERRORLEVEL%
