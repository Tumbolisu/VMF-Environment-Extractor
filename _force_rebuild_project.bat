@echo off
echo [BAT] Removing Old Files...
make clean  ||  goto :errorclean
echo [BAT] Building Program...
make        ||  goto :errormake
PAUSE
exit


:errorclean
echo [BAT] An error occured while removing old files! Exit code: %ERRORLEVEL%
PAUSE
exit /b %ERRORLEVEL%

:errormake
echo [BAT] An error occured while building the project! Exit code: %ERRORLEVEL%
PAUSE
exit /b %ERRORLEVEL%
