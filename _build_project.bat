@echo off
echo [BAT] Building Program...
make  ||  goto :errormake
PAUSE
exit

:errormake
echo [BAT] An error occured while building the project! Exit code: %ERRORLEVEL%
PAUSE
exit /b %ERRORLEVEL%
